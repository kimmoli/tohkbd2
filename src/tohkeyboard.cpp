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

#include <mlite5/MGConfItem>

/* Main
 */
Tohkbd::Tohkbd()
{
    interruptsEnabled = false;
    vddEnabled = false;
    stickyCtrl = false;
    capsLockSeq = 0;
    vkbLayoutIsTohkbd = false;
    currentActiveLayout = QString();
    keypadIsPresent = false;
    gpio_fd = -1;
    displayIsOn = false;

    thread = new QThread();
    worker = new Worker();

    worker->moveToThread(thread);
    connect(worker, SIGNAL(gpioInterruptCaptured()), this, SLOT(handleGpioInterrupt()));
    connect(worker, SIGNAL(workRequested()), thread, SLOT(start()));
    connect(thread, SIGNAL(started()), worker, SLOT(doWork()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()), Qt::DirectConnection);

    backlightTimer = new QTimer(this);
    backlightTimer->setInterval(2000);
    backlightTimer->setSingleShot(true);
    connect(backlightTimer, SIGNAL(timeout()), this, SLOT(backlightTimerTimeout()));

    presenceTimer = new QTimer(this);
    presenceTimer->setInterval(2000);
    presenceTimer->setSingleShot(true);
    connect(presenceTimer, SIGNAL(timeout()), this, SLOT(presenceTimerTimeout()));

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

    checkKeypadPresence();

    connect(keymap, SIGNAL(shiftChanged()), this, SLOT(handleShiftChanged()));
    connect(keymap, SIGNAL(ctrlChanged()), this, SLOT(handleCtrlChanged()));
    connect(keymap, SIGNAL(altChanged()), this, SLOT(handleAltChanged()));
    connect(keymap, SIGNAL(symChanged()), this, SLOT(handleSymChanged()));
    connect(keymap, SIGNAL(keyPressed(QList< QPair<int, int> >)), this, SLOT(handleKeyPressed(QList< QPair<int, int> >)));
}


/* Function to set VDD (3.3V for OH)
 */
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
    }

    return vddEnabled;
}


/* Interrupt enable and disable
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

    releaseTohInterrupt(gpio_fd);

    if (state)
    {
        gpio_fd = getTohInterrupt();
        if (gpio_fd < 0)
        {
            printf("failed to enable interrupt\n");
            interruptsEnabled = false;
        }
        else
        {
            worker->requestWork(gpio_fd);
            printf("worker started\n");

            interruptsEnabled = true;
        }
    }

    return interruptsEnabled;
}

/* Handle state change of phone display
 * Turn keyboard backlight on and off
 */
void Tohkbd::handleDisplayStatus(const QDBusMessage& msg)
{
    QList<QVariant> args = msg.arguments();
    const char *turn = qPrintable(args.at(0).toString());

    printf("Display status changed to \"%s\"\n", turn);
    if (strcmp(turn, "on") == 0)
    {
        checkDoWeNeedBacklight();
        displayIsOn = true;
    }
    else if (strcmp(turn, "off") == 0)
    {
        displayIsOn = false;

        if (backlightTimer->isActive())
        {
            backlightTimer->stop();
            tca8424->setLeds(LED_BACKLIGHT_OFF);
        }
    }
}

/* Check is keypad present
 */

bool Tohkbd::checkKeypadPresence()
{
    bool __prev_keypadPresence = keypadIsPresent;
    if (!vddEnabled)
    {
        /* keyboard is being connected to base */
        /* TODO: how to detect keyboard is removed ??? */
        printf("Presence detected, turning power on\n");
        QThread::msleep(150);
        setVddState(true);
        QThread::msleep(150);
        tca8424->reset();
        QThread::msleep(150);
    }
    if (!tca8424->testComms())
    {
        printf("keypad not present, turning power off\n");
        setVddState(false);
        keypadIsPresent = false;
    }
    else
    {
        keypadIsPresent = true;
        presenceTimer->start();

        tca8424->setLeds((stickyCtrl ? LED_SYMLOCK_ON : LED_SYMLOCK_OFF) | ((capsLockSeq == 3) ? LED_CAPSLOCK_ON : LED_CAPSLOCK_OFF));
    }

    if (__prev_keypadPresence != keypadIsPresent)
    {
        emit keyboardConnectedChanged(keypadIsPresent);
        emitKeypadSlideEvent(keypadIsPresent);

        vkbLayoutIsTohkbd = keypadIsPresent;
        changeActiveLayout();
    }

    return keypadIsPresent;
}

/* GPIO interrupt handler.
 * Called when TOHKBD keyboard part is attached to the base, and
 * when there is new input report due a key press.
 */
void Tohkbd::handleGpioInterrupt()
{
    if (!keypadIsPresent)
    {
        checkKeypadPresence();
    }
    else
    {
        keymap->process(tca8424->readInputReport());
    }
}

/* Key press handler. Called from keymap->process() if actual key was pressed
 */
void Tohkbd::handleKeyPressed(QList< QPair<int, int> > keyCode)
{
    presenceTimer->start();

    if (!displayIsOn)
        emitKeypadSlideEvent(true);

    if ((capsLockSeq == 1 || capsLockSeq == 2)) /* Abort caps-lock if other key pressed */
        capsLockSeq = 0;

    checkDoWeNeedBacklight();

    for (int i=0; i<keyCode.count(); i++)
    {
        /* Some of the keys require shift pressed to get correct symbol */
        if (keyCode.at(i).second & FORCE_COMPOSE)
            uinputif->sendUinputKeyPress(KEY_COMPOSE, 1);
        if ((keyCode.at(i).second & FORCE_RIGHTALT) || keymap->symPressed)
            uinputif->sendUinputKeyPress(KEY_RIGHTALT, 1);
        if ((keyCode.at(i).second & FORCE_SHIFT) || keymap->shiftPressed)
            uinputif->sendUinputKeyPress(KEY_LEFTSHIFT, 1);
        if ((keyCode.at(i).second & FORCE_ALT) || keymap->altPressed)
            uinputif->sendUinputKeyPress(KEY_LEFTALT, 1);

        /* Mimic key pressing */
        uinputif->sendUinputKeyPress(keyCode.at(i).first, 1);

        if (keyCode.at(i).second & SYN_BETWEEN)
            uinputif->synUinputDevice();

        uinputif->sendUinputKeyPress(keyCode.at(i).first, 0);

        if ((keyCode.at(i).second & FORCE_ALT) || keymap->altPressed)
            uinputif->sendUinputKeyPress(KEY_LEFTALT, 0);
        if ((keyCode.at(i).second & FORCE_SHIFT) || keymap->shiftPressed)
            uinputif->sendUinputKeyPress(KEY_LEFTSHIFT, 0);
        if ((keyCode.at(i).second & FORCE_RIGHTALT) || keymap->symPressed)
            uinputif->sendUinputKeyPress(KEY_RIGHTALT, 0);
        if (keyCode.at(i).second & FORCE_COMPOSE)
            uinputif->sendUinputKeyPress(KEY_COMPOSE, 0);
    }

    uinputif->synUinputDevice();

    if (stickyCtrl)
    {
        uinputif->sendUinputKeyPress(KEY_LEFTCTRL, 0);
        stickyCtrl = false;
        tca8424->setLeds(LED_SYMLOCK_OFF); /* TODO: Fix correct led when such is in HW */
        printf("Ctrl released automatically\n");
    }
}

/* Shift, Ctrl, Alt and Sym key press and release handlers
 */
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
        tca8424->setLeds(stickyCtrl ? LED_SYMLOCK_ON : LED_SYMLOCK_OFF); /* TODO: Fix correct led when such is in HW */
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

/* Read first line from a text file
 * returns empty QString if failed
 */
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

    return line;
}

/* Read eeprom configuration words to a QList from config_data
 * returns empty QList if failed
 */
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

/* Read ambient light value from phones ALS and decide will the backlight be lit or not
 * Restart timer if timer was already running
 */
void Tohkbd::checkDoWeNeedBacklight()
{
    if (!backlightTimer->isActive())
    {
        if (readOneLineFromFile("/sys/devices/virtual/input/input11/als_lux").toInt() < 5)
        {
            printf("backlight on\n");

            tca8424->setLeds(LED_BACKLIGHT_ON);
            backlightTimer->start();
        }
    }
    else
    {
        backlightTimer->start();
    }
}

/* Backlight timer timeout, will turn backlight off
 */
void Tohkbd::backlightTimerTimeout()
{
    tca8424->setLeds(LED_BACKLIGHT_OFF);
}

/* Change virtual keyboard active layout,
 * uses private: vkbLayoutIsTohkbd
 * true = change to tohkbd.qml
 * false = change to last non-tohkbd layout
 *
 * TODO: A separate daemon for this
 */
void Tohkbd::changeActiveLayout()
{
    printf("Getting current vkb layout\n");

    process = new QProcess();
    QObject::connect(process, SIGNAL(readyRead()), this, SLOT(handleDconfCurrentLayout()));

    process->start("/usr/bin/dconf read /sailfish/text_input/active_layout");
}

void Tohkbd::handleDconfCurrentLayout()
{
    QByteArray ba = process->readAll();

    ba.replace('\n', QString());
    QString __currentActiveLayout = QString(ba);

    printf("Current layout is %s\n", qPrintable(__currentActiveLayout));

    if (__currentActiveLayout.contains("tohkbd.qml") && vkbLayoutIsTohkbd)
    {
        return;
    }
    else if (!__currentActiveLayout.contains("tohkbd.qml"))
    {
        if (__currentActiveLayout.contains("qml"))
            currentActiveLayout = __currentActiveLayout;
    }

    QThread::msleep(100);
    process->terminate();

    process = new QProcess();

    if (vkbLayoutIsTohkbd)
    {
        printf("Changing to tohkbd\n");
        process->start("/usr/bin/dconf write /sailfish/text_input/active_layout \"'tohkbd.qml'\"");
    }
    else if (currentActiveLayout.contains("qml"))
    {
        printf("Changing to %s\n", qPrintable(currentActiveLayout));
        process->start(QString("/usr/bin/dconf write /sailfish/text_input/active_layout \"%1\"").arg(currentActiveLayout));
    }

    QThread::msleep(100);
    process->terminate();

}
/* SW_KEYPAD_SLIDE controls display on/off
 */

void Tohkbd::emitKeypadSlideEvent(bool openKeypad)
{
    printf("SW_KEYPAD_SLIDE %s\n", openKeypad ? "open" : "close");

    uinputif->sendUinputSwitch(SW_KEYPAD_SLIDE, openKeypad ? 1 : 0);
    uinputif->synUinputDevice();
    uinputif->sendUinputSwitch(SW_KEYPAD_SLIDE, openKeypad ? 0 : 1);
    uinputif->synUinputDevice();
}

/* Will check is keyboard still there
 */
void Tohkbd::presenceTimerTimeout()
{
    if (checkKeypadPresence())
    {
        presenceTimer->start();
    }
}

/** DBUS Test methods */

/* dbus-send --system --print-reply --dest=com.kimmoli.tohkbd2 / com.kimmoli.tohkbd2.fakeKeyPress array:byte:0x00,0x00,0x00,0x00,0x00,0xA3,0x00,0x00,0x00,0x00,0x00
 */
void Tohkbd::fakeKeyPress(const QDBusMessage& msg)
{
    QList<QVariant> args = msg.arguments();

    printf("got fake keypress\n");
    keymap->process(args.at(0).toByteArray());
}

/* dbus-send --system --print-reply --dest=com.kimmoli.tohkbd2 / com.kimmoli.tohkbd2.fakeVkbChange boolean:true
 */
void Tohkbd::fakeVkbChange(const QDBusMessage& msg)
{
    QList<QVariant> args = msg.arguments();

    printf("got fake vkb-change\n");

    vkbLayoutIsTohkbd = args.at(0).toBool();
    changeActiveLayout();

}

void Tohkbd::testSwitchEvent(const QDBusMessage &msg)
{
    QList<QVariant> args = msg.arguments();

    if (args.count() == 2)
    {
        printf("setting %d to %d\n", args.at(0).toInt(), args.at(1).toInt());
        uinputif->sendUinputSwitch(args.at(0).toInt(), args.at(1).toInt());
        uinputif->synUinputDevice();
    }
}
