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
#include "defaultSettings.h"
#include "eepromdriver.h"
#include "modifierhandler.h"

static const char *SERVICE = SERVICE_NAME;
static const char *PATH = "/";


/* Main
 */
Tohkbd::Tohkbd(QObject *parent) :
       QObject(parent)
{
    dbusRegistered = false;
    interruptsEnabled = false;
    vddEnabled = false;
    vkbLayoutIsTohkbd = false;
    currentActiveLayout = QString();
    currentOrientationLock = QString();
    keypadIsPresent = false;
    gpio_fd = -1;
    displayIsOn = false;
    keyIsPressed = false;
    keyRepeat = false;
    slideEventEmitted = false;
    taskSwitcherVisible = false;
    selfieLedOn = false;
    gpioInterruptCounter = 0;
    actualSailfishVersion = QString();

    fix_CapsLock = !checkSailfishVersion("1.1.7.0");
    capsLock = false;

    tohkbd2user = new QDBusInterface("com.kimmoli.tohkbd2user", "/", "com.kimmoli.tohkbd2user", QDBusConnection::sessionBus(), this);
    tohkbd2user->setTimeout(2000);

    thread = new QThread();
    worker = new Worker();

    worker->moveToThread(thread);
    connect(worker, SIGNAL(gpioInterruptCaptured()), this, SLOT(handleGpioInterrupt()));
    connect(worker, SIGNAL(workRequested()), thread, SLOT(start()));
    connect(thread, SIGNAL(started()), worker, SLOT(doWork()));
    connect(worker, SIGNAL(finished()), thread, SLOT(quit()), Qt::DirectConnection);

    backlightTimer = new QTimer(this);
    backlightTimer->setSingleShot(true);
    connect(backlightTimer, SIGNAL(timeout()), this, SLOT(backlightTimerTimeout()));

    presenceTimer = new QTimer(this);
    presenceTimer->setInterval(2000);
    presenceTimer->setSingleShot(true);
    connect(presenceTimer, SIGNAL(timeout()), this, SLOT(presenceTimerTimeout()));

    repeatTimer = new QTimer(this);
    repeatTimer->setSingleShot(true);
    connect(repeatTimer, SIGNAL(timeout()), this, SLOT(repeatTimerTimeout()));

    /* do this automatically at startup */
    setVddState(true);
    setInterruptEnable(true);

    uinputif = new UinputIf();
    uinputif->openUinputDevice();

    tca8424 = new tca8424driver(0x3b);
    keymap = new keymapping();

    FKEYS.clear();
    FKEYS.append(KEY_F1);
    FKEYS.append(KEY_F2);
    FKEYS.append(KEY_F3);
    FKEYS.append(KEY_F4);
    FKEYS.append(KEY_F5);
    FKEYS.append(KEY_F6);
    FKEYS.append(KEY_F7);
    FKEYS.append(KEY_F8);
    FKEYS.append(KEY_F9);
    FKEYS.append(KEY_F10);
    FKEYS.append(KEY_F11);
    FKEYS.append(KEY_F12);

    reloadSettings();

    keymap->setLayout(masterLayout);

    if (currentActiveLayout.isEmpty())
        changeActiveLayout(true);

    if (currentOrientationLock.isEmpty())
    {
        changeOrientationLock(true);
        saveOrientation();
    }

    checkKeypadPresence();

    connect(keymap, SIGNAL(shiftChanged()), this, SLOT(handleShiftChanged()));
    connect(keymap, SIGNAL(ctrlChanged()), this, SLOT(handleCtrlChanged()));
    connect(keymap, SIGNAL(altChanged()), this, SLOT(handleAltChanged()));
    connect(keymap, SIGNAL(symChanged()), this, SLOT(handleSymChanged()));
    connect(keymap, SIGNAL(toggleCapsLock()), this, SLOT(toggleCapsLock()));
    connect(keymap, SIGNAL(keyPressed(QList< QPair<int, int> >)), this, SLOT(handleKeyPressed(QList< QPair<int, int> >)));
    connect(keymap, SIGNAL(keyReleased()), this, SLOT(handleKeyReleased()));

}

/* Remove uinput device, stop threads and unregister from dbus
 */
Tohkbd::~Tohkbd()
{
    /* Restore orientation when shutting down */
    if (forceLandscapeOrientation)
    {
        QList<QVariant> args;
        args.append(currentOrientationLock);
        tohkbd2user->callWithArgumentList(QDBus::AutoDetect, "setOrientationLock", args);
    }

    uinputif->closeUinputDevice();

    worker->abort();
    thread->wait();
    delete thread;
    delete worker;

    if (dbusRegistered)
    {
        QDBusConnection connection = QDBusConnection::systemBus();
        connection.unregisterObject(PATH);
        connection.unregisterService(SERVICE);

        printf("tohkbd2: unregistered from dbus systemBus\n");
        tohkbd2user->call(QDBus::AutoDetect, "quit");
    }
}

/* Register to dbus
 */
void Tohkbd::registerDBus()
{
    if (!dbusRegistered)
    {
        // DBus
        QDBusConnection connection = QDBusConnection::systemBus();
        if (!connection.registerService(SERVICE))
        {
            QCoreApplication::quit();
            return;
        }

        if (!connection.registerObject(PATH, this))
        {
            QCoreApplication::quit();
            return;
        }
        dbusRegistered = true;

        printf("tohkbd2: succesfully registered to dbus systemBus \"%s\"\n", SERVICE);
    }
}

/* quit
 */
void Tohkbd::quit()
{
    printf("Requested to quit tohkbd2 daemon from dbus\n");
    QCoreApplication::quit();
}

/* Version
 */
QString Tohkbd::getVersion()
{
    return QString(APPVERSION);
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
        controlLeds(true);
        checkDoWeNeedBacklight();
        displayIsOn = true;
        slideEventEmitted = false;
    }
    else if (strcmp(turn, "off") == 0)
    {
        displayIsOn = false;

        backlightTimer->stop();
        controlLeds(false);
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

    tca8424driver::PresenceResult res = tca8424->testComms();

    if (res == tca8424driver::DetectFail)
    {
        printf("keypad not present, turning power off\n");
        setVddState(false);

        if (keypadIsPresent)
        {
            keyboardConnectedNotification(false);
            presenceTimer->stop();
            handleKeyReleased();
        }

        keypadIsPresent = false;
    }
    else
    {
        if (!keypadIsPresent)
        {
            keyboardConnectedNotification(true);
            controlLeds(true);
            checkDoWeNeedBacklight();
            checkEEPROM();
        }
        else if (res == tca8424driver::NoKeyPressedSinceReset && displayIsOn)
        {
            /* Keyboard power interrupt shortly? refresh leds just in case */
            controlLeds(true);
        }

        presenceTimer->start();
        keypadIsPresent = true;
    }

    if (__prev_keypadPresence != keypadIsPresent)
    {
        emit keyboardConnectedChanged(keypadIsPresent);
        emitKeypadSlideEvent(keypadIsPresent);

        vkbLayoutIsTohkbd = keypadIsPresent;
        changeActiveLayout();
        if (forceLandscapeOrientation)
            changeOrientationLock();
    }

    return keypadIsPresent;
}

/* Controls backlight and status leds
 * restore = true - restores previous state according to internal states
 * restore = false - turns leds off
 */
void Tohkbd::controlLeds(bool restore)
{
    if (!vddEnabled) /* No power applied, get out from here */
        return;

    if (restore)
    {
        /* If backlight was on, do not turn it off */
        int i = LED_CAPSLOCK_OFF | LED_SYMLOCK_OFF | LED_SELFIE_OFF;

        if (((keymap->shift->mode == modifierHandler::Sticky) && keymap->shift->pressed)
          || ((keymap->shift->mode == modifierHandler::Lock) && keymap->shift->locked))
            i |= LED_SYMLOCK_ON;

        if (((keymap->ctrl->mode == modifierHandler::Sticky) && keymap->ctrl->pressed)
          || ((keymap->ctrl->mode == modifierHandler::Lock) && keymap->ctrl->locked))
            i |= LED_SYMLOCK_ON;

        if (((keymap->alt->mode == modifierHandler::Sticky) && keymap->alt->pressed)
          || ((keymap->alt->mode == modifierHandler::Lock) && keymap->alt->locked))
            i |= LED_SYMLOCK_ON;

        if (((keymap->sym->mode == modifierHandler::Sticky) && keymap->sym->pressed)
          || ((keymap->sym->mode == modifierHandler::Lock) && keymap->sym->locked))
            i |= LED_SYMLOCK_ON;

        if (capsLock)
            i |= LED_CAPSLOCK_ON;

        if (selfieLedOn)
            i |= LED_SELFIE_ON;

        if (forceBacklightOn)
            i |= LED_BACKLIGHT_ON;

        tca8424->setLeds(i);
    }
    else
    {
        tca8424->setLeds(LED_ALL_OFF);
    }
}

/* GPIO interrupt handler.
 * Called when TOHKBD keyboard part is attached to the base, and
 * when there is new input report due a key press.
 */
void Tohkbd::handleGpioInterrupt()
{
    if (gpioInterruptCounter == 0)
    {
        gpioInterruptFloodDetect.start();
    }

    /* If there are > 100 interrupts within one sec, there must be something wrong */
    if (++gpioInterruptCounter >= 100)
    {
        int tmsec = gpioInterruptFloodDetect.elapsed();

        printf("100 interrputs in %d ms\n", tmsec);

        if (tmsec < 1000)
        {
            /* Turn keyboard off */
            setVddState(false);
        }

        gpioInterruptCounter = 0;
    }

    if (!keypadIsPresent)
    {
        checkKeypadPresence();
    }
    else
    {
        int retries = 3;
        do
        {
            QByteArray r = tca8424->readInputReport();
            if (!r.isEmpty())
            {
                presenceTimer->start();

                /* Process report only if it has correct length of 11 */
                if (r.at(0) == 0x0b && r.at(1) == 0x00)
                    keymap->process(r);

                /* Check if interrupt line is still down, there is another report for us then */
                if (readOneLineFromFile("/sys/class/gpio/gpio" GPIO_INT "/value") == "0")
                {
                    printf("Interrupt is still low. Reread report.\n");
                    retries--;
                }
                else
                {
                    retries = -1;
                }
            }
            else
            {
                printf("Something wrong here now, retrying... %d\n", retries);
                retries--;
                QThread::msleep(100);
            }
        } while (retries > 0);

        if (retries == 0) /* Did we loose keyboard */
            checkKeypadPresence();
    }
}

/* Key press handler. Called from keymap->process() if actual key was pressed
 */
void Tohkbd::handleKeyPressed(QList< QPair<int, int> > keyCode)
{
    bool processAllKeys = true;

    if (!displayIsOn && !slideEventEmitted)
    {
        emitKeypadSlideEvent(true);
        /* slideEventEmitted is used to limit events to just one
         * emitted once when key pressed while display is off
         * it is reset when display turns on. */
        slideEventEmitted = true;
    }

    checkDoWeNeedBacklight();

    /* alt+TAB is the task-switcher */
    if (keymap->alt->pressed && keyCode.at(0).first == KEY_TAB)
    {
        if (!taskSwitcherVisible)
        {
            /* show taskswitcher and advance one app */
            taskSwitcherVisible = true;
            tohkbd2user->call(QDBus::AutoDetect, "nextAppTaskSwitcher");
            tohkbd2user->call(QDBus::AutoDetect, "showTaskSwitcher");
        }
        else
        {
            /* Toggle to next app */
            tohkbd2user->call(QDBus::AutoDetect, "nextAppTaskSwitcher");
        }
        /* Don't process further */
        keyIsPressed = true;
        return;
    }

    /* Check custom key mappings */

    if (keyCode.at(0).first > KEY_MAX)
    {
        switch (keyCode.at(0).first)
        {
            /* Sym-Int takes a screenshot */
            case KEY_TOH_SCREENSHOT:
                tohkbd2user->call(QDBus::AutoDetect, "takeScreenShot");
                break;

            /* Sym-Del toggles "selfie" led */
            case KEY_TOH_SELFIE:
                selfieLedOn = !selfieLedOn;
                tca8424->setLeds(selfieLedOn ? LED_SELFIE_ON : LED_SELFIE_OFF);
                break;

            case KEY_TOH_NEWEMAIL:
                {
                QDBusMessage m = QDBusMessage::createMethodCall("com.jolla.email.ui",
                                                                "/com/jolla/email/ui",
                                                                "",
                                                                "mailto" );

                QList<QVariant> args;
                args.append(QStringList() << "mailto:");
                m.setArguments(args);

                QDBusConnection::sessionBus().send(m);
                }
                break;

            default:
                break;
        }

        /* Don't process further */
        keyIsPressed = true;
        return;
    }

    /* if F1...F12 key is pressed then launch detached process */

    if (FKEYS.contains(keyCode.at(0).first))
    {
        QString cmd = applicationShortcuts[keyCode.at(0).first];

        if (!cmd.isEmpty())
        {
            printf("Requesting user daemon to start %s\n", qPrintable(cmd));

            QList<QVariant> args;
            args.append(cmd);
            tohkbd2user->callWithArgumentList(QDBus::AutoDetect, "launchApplication", args);

            /* Don't process further */
            keyIsPressed = true;
            return;
        }
    }

    /* Catch ctrl-alt-del (Works only from left ctrl or stickies) */

    if (keymap->alt->pressed && keymap->ctrl->pressed && keyCode.at(0).first == KEY_DELETE)
    {
        printf("Requesting user daemon to reboot with remorse.\n");

        QList<QVariant> args;
        args.append(QString(ACTION_REBOOT_REMORSE));
        tohkbd2user->callWithArgumentList(QDBus::AutoDetect, "actionWithRemorse", args);

        keyIsPressed = true;
        return;
    }

    /* Catch ctrl-alt-backspace to restart lipstick (Works only from left ctrl or stickies) */

    if (keymap->alt->pressed && keymap->ctrl->pressed && keyCode.at(0).first == KEY_BACKSPACE)
    {
        printf("Requesting user daemon to restart lipstick with remorse.\n");

        QList<QVariant> args;
        args.append(QString(ACTION_RESTART_LIPSTICK_REMORSE));
        tohkbd2user->callWithArgumentList(QDBus::AutoDetect, "actionWithRemorse", args);

        keyIsPressed = true;
        return;
    }

    if (processAllKeys)
    {
        for (int i=0; i<keyCode.count(); i++)
        {
            bool tweakCapsLock = false;
            if (fix_CapsLock)
                tweakCapsLock = (capsLock == 3 && ((keyCode.at(i).first >= KEY_Q && keyCode.at(i).first <= KEY_P)
                                                   || (keyCode.at(i).first >= KEY_A && keyCode.at(i).first <= KEY_L)
                                                   || (keyCode.at(i).first >= KEY_Z && keyCode.at(i).first <= KEY_M) ));

            /* Some of the keys require shift pressed to get correct symbol */
            if (keyCode.at(i).second & FORCE_COMPOSE)
                uinputif->sendUinputKeyPress(KEY_COMPOSE, 1);
            if ((keyCode.at(i).second & FORCE_RIGHTALT))
                uinputif->sendUinputKeyPress(KEY_RIGHTALT, 1);
            if ((keyCode.at(i).second & FORCE_SHIFT) || tweakCapsLock)
                uinputif->sendUinputKeyPress(KEY_LEFTSHIFT, 1);
            if ((keyCode.at(i).second & FORCE_ALT))
                uinputif->sendUinputKeyPress(KEY_LEFTALT, 1);
            if ((keyCode.at(i).second & FORCE_CTRL))
                uinputif->sendUinputKeyPress(KEY_LEFTCTRL, 1);

            /* Mimic key pressing */
            uinputif->sendUinputKeyPress(keyCode.at(i).first, 1);
            QThread::msleep(KEYREPEAT_RATE);
            uinputif->sendUinputKeyPress(keyCode.at(i).first, 0);

            if ((keyCode.at(i).second & FORCE_CTRL))
                uinputif->sendUinputKeyPress(KEY_LEFTCTRL, 0);
            if ((keyCode.at(i).second & FORCE_ALT))
                uinputif->sendUinputKeyPress(KEY_LEFTALT, 0);
            if ((keyCode.at(i).second & FORCE_SHIFT) || tweakCapsLock)
                uinputif->sendUinputKeyPress(KEY_LEFTSHIFT, 0);
            if ((keyCode.at(i).second & FORCE_RIGHTALT))
                uinputif->sendUinputKeyPress(KEY_RIGHTALT, 0);
            if (keyCode.at(i).second & FORCE_COMPOSE)
                uinputif->sendUinputKeyPress(KEY_COMPOSE, 0);
        }
    }

    uinputif->synUinputDevice();

    /* store keycode for repeat */
    lastKeyCode = keyCode;

    /* Repeat delay first, then repeat rate */
    repeatTimer->start(keyRepeat ? (keyRepeatRate-(KEYREPEAT_RATE-1)) : keyRepeatDelay);
    keyIsPressed = true;
}

/* Key repeat timer timeout. Re-handle key pressed, if key still pressed.
 */
void Tohkbd::repeatTimerTimeout()
{
    keyRepeat = true;
    handleKeyPressed(lastKeyCode);
}

/* Stop repeat timer when key released
 */
void Tohkbd::handleKeyReleased()
{
    repeatTimer->stop();
    keyRepeat = false;

    if (keyIsPressed)
        keymap->releaseStickyModifiers();

    keyIsPressed = false;
}

/* Shift, Ctrl, Alt and Sym key press and release handlers
 */
void Tohkbd::handleShiftChanged()
{
    /* Release capslock if shift is pressed, to aVOID tHIS */
    if (capsLock && keymap->shift->pressed)
        toggleCapsLock();

    controlLeds(true);
    checkDoWeNeedBacklight();

    uinputif->sendUinputKeyPress(KEY_LEFTSHIFT, keymap->shift->pressed ? 1 : 0);
    QThread::msleep(KEYREPEAT_RATE);
    uinputif->synUinputDevice();
}

void Tohkbd::handleCtrlChanged()
{
    controlLeds(true);
    checkDoWeNeedBacklight();

    uinputif->sendUinputKeyPress(KEY_LEFTCTRL, keymap->ctrl->pressed ? 1 : 0);
    QThread::msleep(KEYREPEAT_RATE);
    uinputif->synUinputDevice();
}

void Tohkbd::handleAltChanged()
{
    controlLeds(true);
    checkDoWeNeedBacklight();

    uinputif->sendUinputKeyPress(KEY_LEFTALT, keymap->alt->pressed ? 1 : 0);
    QThread::msleep(KEYREPEAT_RATE);
    uinputif->synUinputDevice();

    if (!keymap->alt->pressed && taskSwitcherVisible)
    {
        /* hide taskswitcher when alt is released
         * this will also activate selected application */
        tohkbd2user->call(QDBus::AutoDetect, "hideTaskSwitcher");
        taskSwitcherVisible = false;
    }
}

void Tohkbd::handleSymChanged()
{
    controlLeds(true);
    checkDoWeNeedBacklight();
}


void Tohkbd::toggleCapsLock()
{
    uinputif->sendUinputKeyPress(KEY_CAPSLOCK, 1);
    QThread::msleep(KEYREPEAT_RATE);
    uinputif->sendUinputKeyPress(KEY_CAPSLOCK, 0);
    uinputif->synUinputDevice();

    capsLock = !capsLock;

    if (capsLock)
    {
        tca8424->setLeds(LED_CAPSLOCK_ON);
        printf("CapsLock on\n");
    }
    else
    {
        tca8424->setLeds(LED_CAPSLOCK_OFF);
        printf("CapsLock off\n");
    }
}


/* Read first line from a text file
 * returns empty QString if failed
 */
QString Tohkbd::readOneLineFromFile(const QString &fileName)
{
    QString line;

    QFile inputFile( fileName );

    if ( inputFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
       QTextStream in( &inputFile );
       line = in.readLine();
       inputFile.close();
    }

    return line;
}


/* Read ambient light value from phones ALS and decide will the backlight be lit or not
 * Restart timer if timer was already running
 */
void Tohkbd::checkDoWeNeedBacklight()
{
    if (!vddEnabled) /* No power applied, get out from here */
        return;

    if (forceBacklightOn)
    {
        tca8424->setLeds(LED_BACKLIGHT_ON);
    }
    else if (backlightEnabled)
    {
        if (!backlightTimer->isActive())
        {
            if (readOneLineFromFile("/sys/devices/virtual/input/input11/als_lux").toInt() < backlightLuxThreshold)
            {
                printf("backlight on\n");

                tca8424->setLeds(LED_BACKLIGHT_ON);
                backlightTimer->start();
            }
            else
            {
                tca8424->setLeds(LED_BACKLIGHT_OFF);
            }
        }
        else
        {
            backlightTimer->start();
        }
    } /* No backlight */
    else
    {
        backlightTimer->stop();
        tca8424->setLeds(LED_BACKLIGHT_OFF);
    }
}

/* Backlight timer timeout, will turn backlight off if not forced on
 */
void Tohkbd::backlightTimerTimeout()
{
    if (!vddEnabled) /* No power applied, get out from here */
        return;

    if (!forceBacklightOn)
        tca8424->setLeds(LED_BACKLIGHT_OFF);
}

/* Change virtual keyboard active layout,
 * uses private: vkbLayoutIsTohkbd
 * true = change to harbour-tohkbd2.qml
 * false = change to last non-tohkbd layout
 */
void Tohkbd::changeActiveLayout(bool justGetIt)
{
    QString __currentActiveLayout = tohkbd2user->call(QDBus::AutoDetect, "getActiveLayout").arguments().at(0).toString();

    printf("Current layout is %s\n", qPrintable(__currentActiveLayout));

    if (__currentActiveLayout.contains("harbour-tohkbd2.qml") && vkbLayoutIsTohkbd)
    {
        return;
    }
    else if (!__currentActiveLayout.contains("harbour-tohkbd2.qml"))
    {
        if (__currentActiveLayout.contains("qml"))
        {
            if (__currentActiveLayout != currentActiveLayout)
            {
                currentActiveLayout = __currentActiveLayout;
                saveActiveLayout();
            }
        }
    }

    if (justGetIt)
        return;

    if (vkbLayoutIsTohkbd)
    {
        printf("Changing to tohkbd\n");
        QList<QVariant> args;
        args.append("harbour-tohkbd2.qml");
        tohkbd2user->callWithArgumentList(QDBus::AutoDetect, "setActiveLayout", args);
    }
    else if (currentActiveLayout.contains("qml"))
    {
        printf("Changing to %s\n", qPrintable(currentActiveLayout));
        QList<QVariant> args;
        args.append(currentActiveLayout);
        tohkbd2user->callWithArgumentList(QDBus::AutoDetect, "setActiveLayout", args);
    }
}

/* Orientation lock control
 * If enabled in settings, landscape mode is forced when tohkbd is slided out
 */
void Tohkbd::changeOrientationLock(bool justGetIt)
{
    QString __currentOrientationLock = tohkbd2user->call(QDBus::AutoDetect, "getOrientationLock").arguments().at(0).toString();

    printf("Current orientation lock is %s\n", qPrintable(__currentOrientationLock));

    /* Keyboard is now attached, store original orientation */
    if (keypadIsPresent || justGetIt)
        currentOrientationLock = __currentOrientationLock;

    if (justGetIt)
        return;

    if (keypadIsPresent)
    {
        /* Force to landscape if keypad is present */
        QList<QVariant> args;
        args.append("landscape");
        tohkbd2user->callWithArgumentList(QDBus::AutoDetect, "setOrientationLock", args);
    }
    else if (!currentOrientationLock.isEmpty())
    {
        /* Or return the previous value, if we have one */
        QList<QVariant> args;
        args.append(currentOrientationLock);
        tohkbd2user->callWithArgumentList(QDBus::AutoDetect, "setOrientationLock", args);
    }
}

/* SW_KEYPAD_SLIDE controls display on/off
 * Note, this requires mce 1.37.1 + wip_configurable_evdev
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

/* Read daemon settings
 */
void Tohkbd::reloadSettings()
{
    QSettings settings(QSettings::SystemScope, "harbour-tohkbd2", "tohkbd2");

    settings.beginGroup("vkb");
    currentActiveLayout = settings.value("activeLayout", "").toString();
    settings.endGroup();

    settings.beginGroup("applicationshortcuts");

    applicationShortcuts[KEY_F1] = settings.value(QString("KEY_F1"), SHORTCUT_KEY_F1).toString();
    applicationShortcuts[KEY_F2] = settings.value(QString("KEY_F2"), SHORTCUT_KEY_F2).toString();
    applicationShortcuts[KEY_F3] = settings.value(QString("KEY_F3"), SHORTCUT_KEY_F3).toString();
    applicationShortcuts[KEY_F4] = settings.value(QString("KEY_F4"), SHORTCUT_KEY_F4).toString();
    applicationShortcuts[KEY_F5] = settings.value(QString("KEY_F5"), SHORTCUT_KEY_F5).toString();
    applicationShortcuts[KEY_F6] = settings.value(QString("KEY_F6"), SHORTCUT_KEY_F6).toString();
    applicationShortcuts[KEY_F7] = settings.value(QString("KEY_F7"), SHORTCUT_KEY_F7).toString();
    applicationShortcuts[KEY_F8] = settings.value(QString("KEY_F8"), SHORTCUT_KEY_F8).toString();
    applicationShortcuts[KEY_F9] = settings.value(QString("KEY_F9"), SHORTCUT_KEY_F9).toString();
    applicationShortcuts[KEY_F10] = settings.value(QString("KEY_F10"), SHORTCUT_KEY_F10).toString();
    applicationShortcuts[KEY_F11] = settings.value(QString("KEY_F11"), SHORTCUT_KEY_F11).toString();
    applicationShortcuts[KEY_F12] = settings.value(QString("KEY_F12"), SHORTCUT_KEY_F12).toString();

    for (int i = 0 ; i<FKEYS.length() ; i++)
    {
        printf("Shortcut F%d : %s\n", i+1, qPrintable(applicationShortcuts[FKEYS.at(i)]));
        /* Write them back, as we need default values there in settings app */
        settings.setValue(QString("KEY_F%1").arg(i+1), applicationShortcuts[FKEYS.at(i)]);
    }
    settings.endGroup();

    settings.beginGroup("orientation");
    currentOrientationLock = settings.value("originalOrientation", QString()).toString();
    settings.endGroup();

    settings.beginGroup("layoutSettings");
    masterLayout = settings.value("masterLayout", QString(MASTER_LAYOUT)).toString();
    settings.endGroup();

    settings.beginGroup("generalsettings");
    backlightTimer->setInterval(settings.value("backlightTimeout", BACKLIGHT_TIMEOUT).toInt());
    backlightLuxThreshold = settings.value("backlightLuxThreshold", BACKLIGHT_LUXTHRESHOLD).toInt();
    backlightEnabled = settings.value("backlightEnabled", BACKLIGHT_ENABLED).toBool();
    keyRepeatDelay = settings.value("keyRepeatDelay", KEYREPEAT_DELAY).toInt();
    keyRepeatRate = settings.value("keyRepeatRate", KEYREPEAT_RATE).toInt();

    if (settings.value("stickyShiftEnabled", STICKY_SHIFT_ENABLED).toBool())
        keymap->shift->setMode(modifierHandler::Sticky);
    else if (settings.value("lockingShiftEnabled", LOCKING_SHIFT_ENABLED).toBool())
        keymap->shift->setMode(modifierHandler::Lock);
    else
        keymap->shift->setMode(modifierHandler::Normal);

    if (settings.value("stickyCtrlEnabled", STICKY_CTRL_ENABLED).toBool())
        keymap->ctrl->setMode(modifierHandler::Sticky);
    else if (settings.value("lockingCtrlEnabled", LOCKING_CTRL_ENABLED).toBool())
        keymap->ctrl->setMode(modifierHandler::Lock);
    else
        keymap->ctrl->setMode(modifierHandler::Normal);

    if (settings.value("stickyAltEnabled", STICKY_ALT_ENABLED).toBool())
        keymap->alt->setMode(modifierHandler::Sticky);
    else if (settings.value("lockingAltEnabled", LOCKING_ALT_ENABLED).toBool())
        keymap->alt->setMode(modifierHandler::Lock);
    else
        keymap->alt->setMode(modifierHandler::Normal);

    if (settings.value("stickySymEnabled", STICKY_SYM_ENABLED).toBool())
        keymap->sym->setMode(modifierHandler::Sticky);
    else if (settings.value("lockingSymEnabled", LOCKING_SYM_ENABLED).toBool())
        keymap->sym->setMode(modifierHandler::Lock);
    else
        keymap->sym->setMode(modifierHandler::Normal);

    forceLandscapeOrientation = settings.value("forceLandscapeOrientation", FORCE_LANDSCAPE_ORIENTATION).toBool();
    forceBacklightOn = settings.value("forceBacklightOn", FORCE_BACKLIGHT_ON).toBool();
    settings.endGroup();
}

/* Save activeLayout to settings
 */
void Tohkbd::saveActiveLayout()
{
    QSettings settings(QSettings::SystemScope, "harbour-tohkbd2", "tohkbd2");
    settings.beginGroup("vkb");
    settings.setValue("activeLayout", currentActiveLayout);
    settings.endGroup();
}

/* Save current orientation to settings
 */
void Tohkbd::saveOrientation()
{
    QSettings settings(QSettings::SystemScope, "harbour-tohkbd2", "tohkbd2");
    settings.beginGroup("orientation");
    settings.setValue("originalOrientation", currentOrientationLock);
    settings.endGroup();
}

/* set shortcut path (dbus call method)
 */
void Tohkbd::setShortcut(const QString &key, const QString &appPath)
{
    printf("shortcut %s = %s\n", qPrintable(key), qPrintable(appPath));

    if (key.startsWith("F") && appPath.contains(".desktop"))
    {
        QSettings settings(QSettings::SystemScope, "harbour-tohkbd2", "tohkbd2");

        settings.beginGroup("applicationshortcuts");
        settings.setValue(QString("KEY_%1").arg(key), appPath);

        for (int i = 0 ; i<FKEYS.length() ; i++)
        {
            applicationShortcuts[FKEYS.at(i)] = settings.value(QString("KEY_F%1").arg(i+1), applicationShortcuts[FKEYS.at(i)]).toString();
        }

        settings.endGroup();
    }
}

/* Restore all shortcuts to default values
 */
void Tohkbd::setShortcutsToDefault()
{
    QSettings settings(QSettings::SystemScope, "harbour-tohkbd2", "tohkbd2");
    settings.beginGroup("applicationshortcuts");

    applicationShortcuts[KEY_F1] = SHORTCUT_KEY_F1;
    applicationShortcuts[KEY_F2] = SHORTCUT_KEY_F2;
    applicationShortcuts[KEY_F3] = SHORTCUT_KEY_F3;
    applicationShortcuts[KEY_F4] = SHORTCUT_KEY_F4;
    applicationShortcuts[KEY_F5] = SHORTCUT_KEY_F5;
    applicationShortcuts[KEY_F6] = SHORTCUT_KEY_F6;
    applicationShortcuts[KEY_F7] = SHORTCUT_KEY_F7;
    applicationShortcuts[KEY_F8] = SHORTCUT_KEY_F8;
    applicationShortcuts[KEY_F9] = SHORTCUT_KEY_F9;
    applicationShortcuts[KEY_F10] = SHORTCUT_KEY_F10;
    applicationShortcuts[KEY_F11] = SHORTCUT_KEY_F11;
    applicationShortcuts[KEY_F12] = SHORTCUT_KEY_F12;

    for (int i = 0 ; i<FKEYS.length() ; i++)
    {
        settings.setValue(QString("KEY_F%1").arg(i+1), applicationShortcuts[FKEYS.at(i)]);
    }
    settings.endGroup();
}

/* Set integer setting and save it to settings
 */
void Tohkbd::setSettingInt(const QString &key, const int &value)
{
    QSettings settings(QSettings::SystemScope, "harbour-tohkbd2", "tohkbd2");

    if (key == "backlightTimeout" && value >= 100 && value <= 5000)
    {
        settings.beginGroup("generalsettings");
        settings.setValue("backlightTimeout", value);
        backlightTimer->setInterval(value);
        settings.endGroup();
    }
    else if (key == "backlightLuxThreshold" && value >= 1 && value <= 50)
    {
        settings.beginGroup("generalsettings");
        settings.setValue("backlightLuxThreshold", value);
        backlightLuxThreshold = value;
        settings.endGroup();
    }
    else if (key == "keyRepeatDelay" && value >= 50 && value <= 500)
    {
        settings.beginGroup("generalsettings");
        settings.setValue("keyRepeatDelay", value);
        keyRepeatDelay = value;
        settings.endGroup();
    }
    else if (key == "keyRepeatRate" && value >= 25 && value <= 100)
    {
        settings.beginGroup("generalsettings");
        settings.setValue("keyRepeatRate", value);
        keyRepeatRate = value;
        settings.endGroup();
    }
    else if (key == "backlightEnabled" && (value == 0 || value == 1))
    {
        settings.beginGroup("generalsettings");
        settings.setValue("backlightEnabled", (value == 1));
        backlightEnabled = (value == 1);
        settings.endGroup();
        checkDoWeNeedBacklight();
    }
    else if (key == "forceBacklightOn" && (value == 0 || value == 1))
    {
        settings.beginGroup("generalsettings");
        settings.setValue("forceBacklightOn", (value == 1));
        forceBacklightOn = (value == 1);
        settings.endGroup();
        checkDoWeNeedBacklight();
    }
    else if (key == "forceLandscapeOrientation" && (value == 0 || value == 1))
    {
        settings.beginGroup("generalsettings");
        settings.setValue("forceLandscapeOrientation", (value == 1));
        forceLandscapeOrientation = (value == 1);
        settings.endGroup();

        if (value == 0 && !currentOrientationLock.isEmpty())
        {
            QList<QVariant> args;
            args.append(currentOrientationLock);
            tohkbd2user->callWithArgumentList(QDBus::AutoDetect, "setOrientationLock", args);
        }
        else if (value == 1 && keypadIsPresent)
        {
            QList<QVariant> args;
            args.append("landscape");
            tohkbd2user->callWithArgumentList(QDBus::AutoDetect, "setOrientationLock", args);
        }
    }
    else if (key == "stickyShiftEnabled" && (value == 0 || value == 1))
    {
        settings.beginGroup("generalsettings");
        settings.setValue("stickyShiftEnabled", (value == 1));
        keymap->shift->setMode((value == 1) ? modifierHandler::Sticky : modifierHandler::Normal);
        settings.endGroup();
    }
    else if (key == "stickyCtrlEnabled" && (value == 0 || value == 1))
    {
        settings.beginGroup("generalsettings");
        settings.setValue("stickyCtrlEnabled", (value == 1));
        keymap->ctrl->setMode((value == 1) ? modifierHandler::Sticky : modifierHandler::Normal);
        settings.endGroup();
    }
    else if (key == "stickyAltEnabled" && (value == 0 || value == 1))
    {
        settings.beginGroup("generalsettings");
        settings.setValue("stickyAltEnabled", (value == 1));
        keymap->alt->setMode((value == 1) ? modifierHandler::Sticky : modifierHandler::Normal);
        settings.endGroup();
    }
    else if (key == "stickySymEnabled" && (value == 0 || value == 1))
    {
        settings.beginGroup("generalsettings");
        settings.setValue("stickySymEnabled", (value == 1));
        keymap->sym->setMode((value == 1) ? modifierHandler::Sticky : modifierHandler::Normal);
        settings.endGroup();
    }
    else if (key == "lockingShiftEnabled" && (value == 0 || value == 1))
    {
        settings.beginGroup("generalsettings");
        settings.setValue("lockingShiftEnabled", (value == 1));
        keymap->shift->setMode((value == 1) ? modifierHandler::Lock : modifierHandler::Normal);
        settings.endGroup();
    }
    else if (key == "lockingCtrlEnabled" && (value == 0 || value == 1))
    {
        settings.beginGroup("generalsettings");
        settings.setValue("lockingCtrlEnabled", (value == 1));
        keymap->ctrl->setMode((value == 1) ? modifierHandler::Lock : modifierHandler::Normal);
        settings.endGroup();
    }
    else if (key == "lockingAltEnabled" && (value == 0 || value == 1))
    {
        settings.beginGroup("generalsettings");
        settings.setValue("lockingAltEnabled", (value == 1));
        keymap->alt->setMode((value == 1) ? modifierHandler::Lock : modifierHandler::Normal);
        settings.endGroup();
    }
    else if (key == "lockingSymEnabled" && (value == 0 || value == 1))
    {
        settings.beginGroup("generalsettings");
        settings.setValue("lockingSymEnabled", (value == 1));
        keymap->sym->setMode((value == 1) ? modifierHandler::Lock : modifierHandler::Normal);
        settings.endGroup();
    }

    keymap->releaseStickyModifiers(true);
}

void Tohkbd::setSettingString(const QString &key, const QString &value)
{
    QSettings settings(QSettings::SystemScope, "harbour-tohkbd2", "tohkbd2");

    if (key == "masterLayout")
    {
        settings.beginGroup("layoutsettings");
        settings.setValue(key, value);
        settings.endGroup();

        masterLayout = value;
        keymap->setLayout(masterLayout);
    }
}

/* Tell user daemon to show notification */
void Tohkbd::keyboardConnectedNotification(bool connected)
{
    QList<QVariant> args;
    args.append(connected);
    tohkbd2user->callWithArgumentList(QDBus::AutoDetect, "showKeyboardConnectionNotification", args);
}

/** DBUS Test methods */

/* dbus-send --system --print-reply --dest=com.kimmoli.tohkbd2 / com.kimmoli.tohkbd2.fakeKeyPress array:byte:0x00,0x00,0x00,0x00,0x00,0xA3,0x00,0x00,0x00,0x00,0x00
 */
void Tohkbd::fakeInputReport(const QByteArray &data)
{
    printf("input report from dbus\n");
    keymap->process(data);
}

/* Checks contents of base and keyboard EEPROM
 */
void Tohkbd::checkEEPROM()
{
    if (tohcoreBind(false)) /* release eeprom from toh-core.0 */
    {
        QByteArray tmp = eepromDriver(0x50).readData(0, 10);
        printf("data read from base eeprom (0x50): ");
        for (int i=0; i<tmp.length() ; i++)
            printf("%02x ", tmp.at(i));
        printf("\n");

        tmp = eepromDriver(0x51).readData(0, 10);
        printf("data read from keyboard eeprom (0x51): ");
        for (int i=0; i<tmp.length() ; i++)
            printf("%02x ", tmp.at(i));
        printf("\n");

        tohcoreBind(true);
        /* todo: Do something with this data */
    }
    else
        printf("failed to read keyboard eeprom\n");
}

/* Binds or unbinds toh-core.0
 */
bool Tohkbd::tohcoreBind(bool bind)
{
    int fd;

    if (bind)
        fd = open("/sys/bus/platform/drivers/toh-core/bind", O_WRONLY);
    else
        fd = open("/sys/bus/platform/drivers/toh-core/unbind", O_WRONLY);

    if (!(fd < 0))
    {
        if (write (fd, "toh-core.0", 10) != 10)
        {
            close(fd);
            printf("toh-core.0 %s failed\n", bind ? "bind" : "unbind");
            return false;
        }
        close(fd);
        return true;
    }
    else
        printf("toh-core.0 %s failed\n", bind ? "bind" : "unbind");

    return false;
}

/*
 * Check that Sailfish version is at least required version
 */
bool Tohkbd::checkSailfishVersion(QString versionToCompare)
{
    QString actualVersion = "0.0.0.0";

    if (actualSailfishVersion.isEmpty())
    {
        QFile inputFile( "/etc/sailfish-release" );

        if ( inputFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
        {
           QTextStream in( &inputFile );

           while (not in.atEnd())
           {
               QString line = in.readLine();
               if (line.startsWith("VERSION_ID="))
               {
                   actualVersion = line.split('=').at(1);
                   break;
               }
           }
           inputFile.close();
        }
        actualSailfishVersion = actualVersion;

        printf("Sailfish version %s\n", qPrintable(actualSailfishVersion));
    }

    QStringList avList = actualSailfishVersion.split(".");
    QStringList vList = versionToCompare.split(".");

    if (avList.size() == 4 && vList.size() == 4)
    {
        long avLong = (avList.at(0).toInt() << 24) | (avList.at(1).toInt() << 16) | (avList.at(2).toInt() << 8) | avList.at(3).toInt();
        long vLong = (vList.at(0).toInt() << 24) | (vList.at(1).toInt() << 16) | (vList.at(2).toInt() << 8) | vList.at(3).toInt();

        return (avLong >= vLong);
    }
    else
    {
        printf("Sailfish version check failed!\n");
        return false;
    }
}
