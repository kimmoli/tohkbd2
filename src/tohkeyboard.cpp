/*
 * (C) 2014 Kimmo Lindholm <kimmo.lindholm@gmail.com> Kimmoli
 *
 * tohkbd daemon, d-bus server call method functions.
 *
 */


#include <QtCore/QCoreApplication>
#include <QDBusMessage>
#include <QThread>

#include <unistd.h>
#include <linux/input.h>
#include <linux/uinput.h>

#include "tohkeyboard.h"
#include "toh.h"
#include "tca8424.h"
#include "uinputif.h"


bool Tohkbd::interruptsEnabled = false;
bool Tohkbd::vddEnabled = false;

int Tohkbd::capsLockSeq = 0;

/* Main */
Tohkbd::Tohkbd()
{
    thread = new QThread();
    worker = new Worker();

    worker->moveToThread(thread);
    connect(worker, SIGNAL(gpioInterruptCaptured()), this, SLOT(handleGpioInterrupt()));
    connect(worker, SIGNAL(workRequested()), thread, SLOT(start()));
    connect(thread, SIGNAL(started()), worker, SLOT(doWork()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()), Qt::DirectConnection);

    /* do this automatically at startup */
    setVddState(true);
    setInterruptEnable(true);

    uinputif = new UinputIf();
    uinputif->openUinputDevice();
}


/* Function to set VDD (3.3V for OH) */
bool Tohkbd::setVddState(bool state)
{
    printf("VDD control request - turn %s\n", state ? "on" : "off");

    if (vddEnabled == state)
    {
        printf("VDD state already %s\n", state ? "on" : "off");
    }
    else if (controlVdd(state) < 0)
    {
        vddEnabled = false;
        printf("VDD control FAILED\n");
    }
    else
    {
        vddEnabled = state;
        printf("VDD control OK\n");
    }

    return vddEnabled;
}


/*
 *    Interrupt stuff
 */


bool Tohkbd::setInterruptEnable(bool state)
{
    printf("Interrupt control request - turn %s\n", state ? "on" : "off");

    if (!vddEnabled)
    {
        printf("Interrupt requested but VDD Not active. Aborting.\n");
        return false;
    }
    else if (state == interruptsEnabled)
    {
        printf("Interrupt state already %s\n", state ? "on" : "off");
        return interruptsEnabled;
    }

    worker->abort();
    thread->wait(); // If the thread is not running, this will immediately return.
    interruptsEnabled = false;
    if (gpio_fd >= 0)
    {
        releaseTohInterrupt(gpio_fd);
        gpio_fd = -1;
    }

    if (state)
    {
        gpio_fd = getTohInterrupt();
        if (gpio_fd < 0)
        {
            printf("failed to enable interrupt (are you root?)\n");
        }
        else
        {
            int fd = tca8424_initComms(TCA_ADDR);
            if (fd < 0)
            {
                releaseTohInterrupt(gpio_fd);
                gpio_fd = -1;
            }
            else
            {
                tca8424_reset(fd);
                tca8424_closeComms(fd);

                worker->requestWork(gpio_fd);
                printf("worker started\n");

                interruptsEnabled = true;
            }
        }
    }

    return interruptsEnabled;
}

void Tohkbd::handleDisplayStatus(const QDBusMessage& msg)
{
    QList<QVariant> args = msg.arguments();
    const char *turn = qPrintable(args.at(0).toString());

    printf("Display status changed to \"%s\"\n", turn);
    if (strcmp(turn, "on") == 0)
    {
        printf("enabling tohkbd\n");
        setVddState(true);
        setInterruptEnable(true);
    }
    else if (strcmp(turn, "off") == 0)
    {
        printf("disabling tohkbd\n");
        setInterruptEnable(false);
        setVddState(false);
    }
}



/*
 *  GPIO interrupt handler
 *
 */

void Tohkbd::handleGpioInterrupt()
{
    static int haveCtrl = 0;
    int fd, code, isShift, isAlt, isCtrl;
    unsigned char inRep[12];
    const char *buf;

    mutex.lock();

    fd = tca8424_initComms(TCA_ADDR);
    if (fd < 0 || tca8424_readInputReport(fd, inRep) < 0)
    {
        if (fd >= 0)
            tca8424_closeComms(fd);
        mutex.unlock();
        return;
    }

    buf = tca8424_processKeyMap(inRep, &code, &isShift, &isAlt, &isCtrl);

    if ((code != 0) && (capsLockSeq == 1 || capsLockSeq == 2)) /* Abort caps-lock if other key pressed */
        capsLockSeq = 0;

    if (code == 0 && isShift && capsLockSeq == 0) /* Shift pressed first time */
        capsLockSeq = 1;
    else if (code == 0 && buf[0]=='!' && capsLockSeq == 1) /* Shift released */
        capsLockSeq = 2;
    else if (code == 0 && isShift && capsLockSeq == 2) /* Shift pressed 2nd time */
    {
        capsLockSeq = 3;
        uinputif->sendUinputKeyPress(KEY_CAPSLOCK, 1);
        uinputif->sendUinputKeyPress(KEY_CAPSLOCK, 0);
        uinputif->synUinputDevice();
        tca8424_leds(fd, 1);
        printf("CapsLock on\n");
    }
    else if (code == 0 && isShift && capsLockSeq == 3) /* Shift pressed 3rd time */
    {
        capsLockSeq = 0;
        uinputif->sendUinputKeyPress(KEY_CAPSLOCK, 1);
        uinputif->sendUinputKeyPress(KEY_CAPSLOCK, 0);
        uinputif->synUinputDevice();
        tca8424_leds(fd, 0);
        printf("CapsLock off\n");
    }
    else if (code == 0 && isCtrl) /* Ctrl pressed */
    {
        haveCtrl ^= 1;
        uinputif->sendUinputKeyPress(KEY_LEFTCTRL, haveCtrl);
        printf("%s\n", haveCtrl ? "Ctrl down" : "Ctrl lifted");
    }

    if (code != 0) /* We resolved what was pressed */
    {
        printf("Key pressed: %s (%d 0x%02x shft=%d alt=%d ctrl=%d)\n",
               buf, code, inRep[5], isShift, isAlt, haveCtrl);

        if (isShift)
            uinputif->sendUinputKeyPress(KEY_LEFTSHIFT, 1);
        uinputif->sendUinputKeyPress(code, 1);
        uinputif->sendUinputKeyPress(code, 0);
        if (isShift)
            uinputif->sendUinputKeyPress(KEY_LEFTSHIFT, 0);
        if (haveCtrl)
        {
            uinputif->sendUinputKeyPress(KEY_LEFTCTRL, 0);
            printf("Ctrl released automatically\n");
            haveCtrl = 0;
        }

        uinputif->synUinputDevice();

    }
    else if (buf[0] != '!')
    {
        printf("UNK Input report: %s %02x%02x%02x%02x%02x %02x%02x%02x%02x%02x%02x\n",
               buf, inRep[0], inRep[1], inRep[2], inRep[3], inRep[4], inRep[5],
               inRep[6], inRep[7], inRep[8], inRep[9], inRep[10]);
    }

    tca8424_closeComms(fd);
    mutex.unlock();
}
