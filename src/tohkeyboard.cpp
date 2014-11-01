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
#include <QTimer>

#include "tohkeyboard.h"
#include "toh.h"
#include "uinputif.h"

/* Main */
Tohkbd::Tohkbd()
{
    interruptsEnabled = false;
    vddEnabled = false;
    stickyCtrl = false;
    capsLockSeq = 0;

    thread = new QThread();
    worker = new Worker();

    worker->moveToThread(thread);
    connect(worker, SIGNAL(gpioInterruptCaptured()), this, SLOT(handleGpioInterrupt()));
    connect(worker, SIGNAL(workRequested()), thread, SLOT(start()));
    connect(thread, SIGNAL(started()), worker, SLOT(doWork()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()), Qt::DirectConnection);

    backlightTimer = new QTimer(this);
    backlightTimer->setInterval(2000);
    connect(backlightTimer, SIGNAL(timeout()), this, SLOT(backlightTimerTimeout()));

    /* do this automatically at startup */
    setVddState(true);
    setInterruptEnable(true);

    uinputif = new UinputIf();
    uinputif->openUinputDevice();

    tca8424 = new tca8424driver(0x3b);
    keymap = new keymapping();

    /*
     * TODO: Change keyboard mapping "getEepromConfig(int number)"
     */
    QList<unsigned int> eepromConfig = readEepromConfig();
    if (eepromConfig.count() > 0)
        printf("eeprom at 0 = %x\n", eepromConfig.at(0));

    connect(keymap, SIGNAL(shiftChanged()), this, SLOT(handleShiftChanged()));
    connect(keymap, SIGNAL(ctrlChanged()), this, SLOT(handleCtrlChanged()));
    connect(keymap, SIGNAL(altChanged()), this, SLOT(handleAltChanged()));
    connect(keymap, SIGNAL(symChanged()), this, SLOT(handleSymChanged()));
    connect(keymap, SIGNAL(keyPressed(int,bool)), this, SLOT(handleKeyPressed(int,bool)));
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

    if (state == interruptsEnabled)
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
            interruptsEnabled = false;
        }
        else
        {

            tca8424->reset();
            worker->requestWork(gpio_fd);
            printf("worker started\n");

            interruptsEnabled = true;
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
        checkDoWeNeedBacklight();
    }
    else if (strcmp(turn, "off") == 0)
    {
        tca8424->setLeds(LED_BACKLIGHT_OFF);
        backlightTimer->stop();
    }
}



/*
 *  GPIO interrupt handler
 *
 */

void Tohkbd::handleGpioInterrupt()
{
    if (!vddEnabled)
    {
        /* keyboard is being connected to base */
        /* TODO: how to detect keyboard is removed ??? */
        QThread::msleep(100);
        setVddState(true);
        QThread::msleep(100);

        if (!tca8424->reset())
            setVddState(false);
    }
    else
    {
        keymap->process(tca8424->readInputReport());
    }
}

void Tohkbd::handleKeyPressed(int keyCode, bool forceShift)
{
    if ((capsLockSeq == 1 || capsLockSeq == 2)) /* Abort caps-lock if other key pressed */
        capsLockSeq = 0;

    checkDoWeNeedBacklight();

    /* Some of the keys require shift pressed to get correct symbol */
    if (forceShift)
        uinputif->sendUinputKeyPress(KEY_LEFTSHIFT, 1);

    /* Mimic key pressing */
    uinputif->sendUinputKeyPress(keyCode, 1);
    uinputif->sendUinputKeyPress(keyCode, 0);

    if (forceShift)
        uinputif->sendUinputKeyPress(KEY_LEFTSHIFT, 0);

    if (stickyCtrl)
    {
        uinputif->sendUinputKeyPress(KEY_LEFTCTRL, 0);
        stickyCtrl = false;
        printf("Ctrl released automatically\n");
    }

    uinputif->synUinputDevice();
}

void Tohkbd::handleShiftChanged()
{
    if (keymap->shiftPressed && capsLockSeq == 0) /* Shift pressed first time */
        capsLockSeq = 1;
    else if (!keymap->shiftPressed && capsLockSeq == 1) /* Shift released */
        capsLockSeq = 2;
    else if (keymap->shiftPressed && capsLockSeq == 2) /* Shift pressed 2nd time */
    {
        capsLockSeq = 3;
        uinputif->sendUinputKeyPress(KEY_CAPSLOCK, 1);
        uinputif->sendUinputKeyPress(KEY_CAPSLOCK, 0);
        uinputif->synUinputDevice();
        tca8424->setLeds(LED_CAPSLOCK_ON);
        printf("CapsLock on\n");
    }
    else if (keymap->shiftPressed && capsLockSeq == 3) /* Shift pressed 3rd time */
    {
        capsLockSeq = 0;
        uinputif->sendUinputKeyPress(KEY_CAPSLOCK, 1);
        uinputif->sendUinputKeyPress(KEY_CAPSLOCK, 0);
        uinputif->synUinputDevice();
        tca8424->setLeds(LED_CAPSLOCK_OFF);
        printf("CapsLock off\n");
    }
}

void Tohkbd::handleCtrlChanged()
{
    if ((capsLockSeq == 1 || capsLockSeq == 2)) /* Abort caps-lock if other key pressed */
        capsLockSeq = 0;

    if (keymap->ctrlPressed)
    {
        stickyCtrl = !stickyCtrl;
        uinputif->sendUinputKeyPress(KEY_LEFTCTRL, stickyCtrl ? 1 : 0);
    }
}

void Tohkbd::handleAltChanged()
{
    if ((capsLockSeq == 1 || capsLockSeq == 2)) /* Abort caps-lock if other key pressed */
        capsLockSeq = 0;

}

void Tohkbd::handleSymChanged()
{
    if ((capsLockSeq == 1 || capsLockSeq == 2)) /* Abort caps-lock if other key pressed */
        capsLockSeq = 0;

}

QString Tohkbd::readOneLineFromFile(QString name)
{
    QString line;

    QFile inputFile( name );

    if ( inputFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
       QTextStream in( &inputFile );
       line = in.readLine();
       inputFile.close();
    }
    else
    {
        line = QString("Error occured.");
    }

    return line;
}

QList<unsigned int> Tohkbd::readEepromConfig()
{
    QList<unsigned int> ret;

    QFile inputFile ( "/sys/devices/platform/toh-core.0/config_data" );
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QByteArray data = inputFile.readAll();

        for (int i=0; i < data.count()/2; i++)
            ret.append((data.at(2*i)<<8) || data.at(2*i+1));

        inputFile.close();
    }

    return ret;
}


void Tohkbd::checkDoWeNeedBacklight()
{
    if (!backlightTimer->isActive())
    {
        if (readOneLineFromFile("/sys/devices/virtual/input/input11/als_lux").toInt() < 5)
        {
            tca8424->setLeds(LED_BACKLIGHT_ON);
            backlightTimer->start();
        }
    }
    else
    {
        backlightTimer->start();
    }
}

void Tohkbd::backlightTimerTimeout()
{
    tca8424->setLeds(LED_BACKLIGHT_OFF);
}
