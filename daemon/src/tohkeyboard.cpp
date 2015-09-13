/*
 * (C) 2014 Kimmo Lindholm <kimmo.lindholm@gmail.com> Kimmoli
 *
 * tohkbd daemon, d-bus server call method functions.
 *
 */


#include <QtCore/QCoreApplication>
#include <QDBusMessage>
#include <QThread>
#include <QTimer>
#include <QHostAddress>
#include <QtSystemInfo/QDeviceInfo>

#include <mce/dbus-names.h>
#include <mce/mode-names.h>

#include <unistd.h>
#include <linux/input.h>
#include <linux/uinput.h>

#include "tohkeyboard.h"
#include "toh.h"
#include "uinputif.h"
#include "defaultSettings.h"
#include "eepromdriver.h"
#include "modifierhandler.h"

static const char *SERVICE = SERVICE_NAME;
static const char *PATH = "/";

QList<int> Tohkbd::FKEYS = QList<int>() << KEY_F1 << KEY_F2 << KEY_F3 << KEY_F4 << KEY_F5 << KEY_F6
                                        << KEY_F7 << KEY_F8 << KEY_F9 << KEY_F10 << KEY_F11 << KEY_F12;


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
    verboseMode = true;
    turnDisplayOffWhenRemoved = false;

    fix_CapsLock = !checkSailfishVersion("1.1.7.0");
    capsLock = false;
}

/* Initialise. Returns false if failed
 */
bool Tohkbd::init()
{
    tohkbd2user = new ComKimmoliTohkbd2userInterface("com.kimmoli.tohkbd2user", "/", QDBusConnection::sessionBus(), this);
    tohkbd2user->setTimeout(2000);

    QString userDaemonVersion;

    printf("waking up user daemon\n");

    userDaemonVersion = tohkbd2user->getVersion();

    /* User daemon needs to be same version as this daemon */
    if (userDaemonVersion == APPVERSION)
    {
        printf("user daemon version %s\n", qPrintable(userDaemonVersion));
    }
    else
    {
        tohkbd2user->quit();
        printf("wrong version of user daemon \"%s\"\n", qPrintable(userDaemonVersion));
        return false;
    }

    connect(tohkbd2user, SIGNAL(physicalLayoutChanged(QString)), this, SLOT(handlePhysicalLayout(QString)));

    tohkbd2settingsui = new ComKimmoliTohkbd2settingsuiInterface("com.kimmoli.tohkbd2settingsui", "/", QDBusConnection::sessionBus(), this);
    tohkbd2settingsui->setTimeout(2000);

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

    uinputevpoll = new UinputEvPoll();
    evpollThread = new QThread();

    uinputevpoll->moveToThread(evpollThread);
    connect(uinputevpoll, SIGNAL(capsLockLedChanged(bool)), this, SLOT(capsLockLedState(bool)));
    connect(uinputevpoll, SIGNAL(pollingRequested()), evpollThread, SLOT(start()));
    connect(evpollThread, SIGNAL(started()), uinputevpoll, SLOT(doPoll()));
    connect(uinputevpoll, SIGNAL(finished()), evpollThread, SLOT(quit()), Qt::DirectConnection);

    uinputevpoll->requestPolling(uinputif->getFd());

    tca8424 = new tca8424driver(0x3b);
    keymap = new keymapping(QString(tohkbd2user->getPathTo("keymaplocation")));

    reloadSettings();

    displayIsOn = getCurrentDisplayState();

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
    connect(keymap, SIGNAL(bogusDetected()), tca8424, SLOT(reset()));
    connect(keymap, SIGNAL(setKeymapLayout(QString)), tohkbd2user, SLOT(setKeymapLayout(QString)));
    connect(keymap, SIGNAL(setKeymapVariant(QString)), tohkbd2user, SLOT(setKeymapVariant(QString)));

    QString currentPhysicalLayout = tohkbd2user->getActivePhysicalLayout();

    printf("physical layout is %s\n", qPrintable(currentPhysicalLayout));
    keymap->setLayout(currentPhysicalLayout);

    return true;
}

/* Remove uinput device, stop threads and unregister from dbus
 */
Tohkbd::~Tohkbd()
{
    /* Restore orientation when shutting down */
    if (forceLandscapeOrientation)
    {
        tohkbd2user->setOrientationLock(currentOrientationLock);
    }

    if (keypadIsPresent)
    {
        keypadIsPresent = false;
        /* keypad was removed if daemon gets killed */
        emitKeypadSlideEvent(keypadIsPresent);

        /* restore vkb when daemon gets killed */
        vkbLayoutIsTohkbd = keypadIsPresent;
        changeActiveLayout();
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
        tohkbd2user->quit();
    }
}

/* Register to dbus
 */
bool Tohkbd::registerDBus()
{
    if (!dbusRegistered)
    {
        // DBus
        QDBusConnection connection = QDBusConnection::systemBus();
        if (!connection.registerService(SERVICE))
        {
            return false;
        }

        if (!connection.registerObject(PATH, this))
        {
            return false;
        }
        dbusRegistered = true;

        printf("tohkbd2: succesfully registered to dbus systemBus \"%s\"\n", SERVICE);
    }
    return dbusRegistered;
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
    if (verboseMode)
        printf("VDD control request - turn %s\n", state ? "on" : "off");

    if (vddEnabled == state)
    {
        if (verboseMode)
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

    if (verboseMode)
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
        if (verboseMode)
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
        if (verboseMode)
            printf("keypad not present, turning power off\n");
        setVddState(false);

        if (keypadIsPresent)
        {
            presenceTimer->stop();
            handleKeyReleased();
        }

        keypadIsPresent = false;
    }
    else
    {
        if (!keypadIsPresent)
        {
            controlLeds(true);
            checkDoWeNeedBacklight();
            checkEEPROM();
        }
        else if (res == tca8424driver::NoKeyPressedSinceReset && displayIsOn)
        {
            /* Keyboard power interrupt shortly? refresh leds just in case */
            controlLeds(true);
            /* ...and release possibly stuck keys */
            handleKeyReleased();
        }

        presenceTimer->start();
        keypadIsPresent = true;
    }

    if (__prev_keypadPresence != keypadIsPresent)
    {
        keyboardConnectedNotification(keypadIsPresent);

        emit keyboardConnectedChanged(keypadIsPresent);
        emitKeypadSlideEvent(keypadIsPresent);

        vkbLayoutIsTohkbd = keypadIsPresent;
        changeActiveLayout();

        if (forceLandscapeOrientation)
        {
            changeOrientationLock();
        }

        if (turnDisplayOffWhenRemoved && !keypadIsPresent && displayIsOn)
        {
            /* if enabled and display is on, turn display off when keyboard is removed */
            QDBusMessage m = QDBusMessage::createMethodCall(MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF, MCE_DISPLAY_OFF_REQ);
            QDBusConnection::systemBus().send(m);
        }
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

        if (((keymap->shift->mode == modifierHandler::Sticky || keymap->shift->mode == modifierHandler::Cycle) && keymap->shift->pressed)
          || ((keymap->shift->mode == modifierHandler::Lock || keymap->shift->mode == modifierHandler::Cycle) && keymap->shift->locked))
            i |= LED_SYMLOCK_ON;

        if (((keymap->ctrl->mode == modifierHandler::Sticky || keymap->ctrl->mode == modifierHandler::Cycle) && keymap->ctrl->pressed)
          || ((keymap->ctrl->mode == modifierHandler::Lock || keymap->ctrl->mode == modifierHandler::Cycle) && keymap->ctrl->locked))
            i |= LED_SYMLOCK_ON;

        if (((keymap->alt->mode == modifierHandler::Sticky || keymap->alt->mode == modifierHandler::Cycle) && keymap->alt->pressed)
          || ((keymap->alt->mode == modifierHandler::Lock || keymap->alt->mode == modifierHandler::Cycle) && keymap->alt->locked))
            i |= LED_SYMLOCK_ON;

        if (((keymap->sym->mode == modifierHandler::Sticky || keymap->sym->mode == modifierHandler::Cycle) && keymap->sym->pressed)
          || ((keymap->sym->mode == modifierHandler::Lock || keymap->sym->mode == modifierHandler::Cycle) && keymap->sym->locked))
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
                {
                    keymap->process(r);
                    retries = -1;
                }
                else
                {
                    retries--;
                }
            }
            else
            {
                if (verboseMode)
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

    /* No need to do this all if we are repeating */
    if (!keyRepeat)
    {
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
                tohkbd2user->nextAppTaskSwitcher();
                tohkbd2user->showTaskSwitcher();
            }
            else
            {
                /* Toggle to next app */
                tohkbd2user->nextAppTaskSwitcher();
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
                    tohkbd2user->takeScreenShot();
                    break;

                /* Sym-Del toggles "selfie" led */
                case KEY_TOH_SELFIE:
                    selfieLedOn = !selfieLedOn;
                    tca8424->setLeds(selfieLedOn ? LED_SELFIE_ON : LED_SELFIE_OFF);
                    break;

                case KEY_TOH_BACKLIGHT:
                    forceBacklightOn = !forceBacklightOn;
                    checkDoWeNeedBacklight();
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
            /* Ctrl-Sym-F1 is Help */
            if (keyCode.at(0).first == KEY_F1 && keymap->ctrl->pressed)
            {
                tohkbd2settingsui->showHelp();

                keyIsPressed = true;
                return;
            }

            QString cmd = applicationShortcuts[keyCode.at(0).first];

            if (!cmd.isEmpty())
            {
                if (verboseMode)
                    printf("Requesting user daemon to start %s\n", qPrintable(cmd));

                tohkbd2user->launchApplication(cmd);

                /* Don't process further */
                keyIsPressed = true;
                return;
            }
        }

        /* Catch ctrl-alt-del (Works only from left ctrl or stickies) */

        if (keymap->alt->pressed && keymap->ctrl->pressed && keyCode.at(0).first == KEY_DELETE)
        {
            if (verboseMode)
                printf("Requesting user daemon to reboot with remorse.\n");

            tohkbd2user->actionWithRemorse(ACTION_REBOOT_REMORSE);

            keyIsPressed = true;
            return;
        }

        /* Catch ctrl-alt-backspace to restart lipstick (Works only from left ctrl or stickies) */

        if (keymap->alt->pressed && keymap->ctrl->pressed && keyCode.at(0).first == KEY_BACKSPACE)
        {
            if (verboseMode)
                printf("Requesting user daemon to restart lipstick with remorse.\n");

            tohkbd2user->actionWithRemorse(ACTION_RESTART_LIPSTICK_REMORSE);

            keyIsPressed = true;
            return;
        }
    }

    if (processAllKeys)
    {
        for (int i=0; i<keyCode.count(); i++)
        {
            bool tweakCapsLock = false;
            if (fix_CapsLock)
                tweakCapsLock = (capsLock && ((keyCode.at(i).first >= KEY_Q && keyCode.at(i).first <= KEY_P)
                                          || (keyCode.at(i).first >= KEY_A && keyCode.at(i).first <= KEY_L)
                                          || (keyCode.at(i).first >= KEY_Z && keyCode.at(i).first <= KEY_M) ));

            if (keyCode.at(i).second & FORCE_COMPOSE)
            {
                uinputif->sendUinputKeyPress(KEY_COMPOSE, 1);
                QThread::msleep(KEYREPEAT_RATE);
                uinputif->sendUinputKeyPress(KEY_COMPOSE, 0);
                uinputif->synUinputDevice();
            }

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
    if (!keyRepeat)
    {
        /* Check is the interrupt stuck down on first repeat timer timeout*/
        if (readOneLineFromFile("/sys/class/gpio/gpio" GPIO_INT "/value") == "0")
        {
            if (verboseMode)
                printf("repeatTimerTimeout: interrupt is active, trying to handle it now.\n");
            handleGpioInterrupt();
            return;
        }
    }

    keyRepeat = true;
    handleKeyPressed(lastKeyCode);

    /* Keep backlight on when repeating if it was turned on when key pressed first time */
    if (backlightTimer->isActive())
        backlightTimer->start();
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
        tohkbd2user->hideTaskSwitcher();
        taskSwitcherVisible = false;
    }
}

void Tohkbd::handleSymChanged()
{
    controlLeds(true);
    checkDoWeNeedBacklight();
}

/* Press caps-lock key once, toggles it state
 * led state is controlled by system through EV_LED
 */
void Tohkbd::toggleCapsLock()
{
    uinputif->sendUinputKeyPress(KEY_CAPSLOCK, 1);
    QThread::msleep(KEYREPEAT_RATE);
    uinputif->sendUinputKeyPress(KEY_CAPSLOCK, 0);
    uinputif->synUinputDevice();
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
                if (verboseMode)
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
    QString __currentActiveLayout = tohkbd2user->getActiveLayout();

    if (verboseMode)
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
        if (verboseMode)
            printf("Changing to tohkbd\n");
        tohkbd2user->setActiveLayout("harbour-tohkbd2.qml");
    }
    else if (currentActiveLayout.contains("qml"))
    {
        if (verboseMode)
            printf("Changing to %s\n", qPrintable(currentActiveLayout));
        tohkbd2user->setActiveLayout(currentActiveLayout);
    }
}

/* Orientation lock control
 * If enabled in settings, landscape mode is forced when tohkbd is slided out
 */
void Tohkbd::changeOrientationLock(bool justGetIt)
{
    QString __currentOrientationLock = tohkbd2user->getOrientationLock();

    if (verboseMode)
        printf("Current orientation lock is %s\n", qPrintable(__currentOrientationLock));

    /* Keyboard is now attached, store original orientation */
    if (keypadIsPresent || justGetIt)
        currentOrientationLock = __currentOrientationLock;

    if (justGetIt)
        return;

    if (keypadIsPresent)
    {
        /* Force to landscape if keypad is present */
        tohkbd2user->setOrientationLock("landscape");
    }
    else if (!currentOrientationLock.isEmpty())
    {
        /* Or return the previous value, if we have one */
        tohkbd2user->setOrientationLock(currentOrientationLock);
    }
}

/* SW_KEYPAD_SLIDE controls display on/off
 * Note, this requires mce 1.37.1 + wip_configurable_evdev
 */

void Tohkbd::emitKeypadSlideEvent(bool openKeypad)
{
    if (verboseMode)
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
        if (readOneLineFromFile("/sys/class/gpio/gpio" GPIO_INT "/value") == "0")
        {
            if (verboseMode)
                printf("checkKeypadPresence: interrupt is active, trying to handle it now.\n");
            handleGpioInterrupt();
        }
    }
}

/* Read daemon settings
 */
void Tohkbd::reloadSettings()
{
    QSettings settings(QSettings::SystemScope, "harbour-tohkbd2", "tohkbd2");

    settings.beginGroup("debug");
    setVerboseMode(settings.value("verboseMode", VERBOSE_MODE_ENABLED).toBool());
    settings.endGroup();

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
        if (verboseMode)
            printf("Shortcut F%d : %s\n", i+1, qPrintable(applicationShortcuts[FKEYS.at(i)]));
        /* Write them back, as we need default values there in settings app */
        settings.setValue(QString("KEY_F%1").arg(i+1), applicationShortcuts[FKEYS.at(i)]);
    }
    settings.endGroup();

    settings.beginGroup("orientation");
    currentOrientationLock = settings.value("originalOrientation", QString()).toString();
    settings.endGroup();

    settings.beginGroup("generalsettings");
    backlightTimer->setInterval(settings.value("backlightTimeout", BACKLIGHT_TIMEOUT).toInt());
    backlightLuxThreshold = settings.value("backlightLuxThreshold", BACKLIGHT_LUXTHRESHOLD).toInt();
    backlightEnabled = settings.value("backlightEnabled", BACKLIGHT_ENABLED).toBool();
    keyRepeatDelay = settings.value("keyRepeatDelay", KEYREPEAT_DELAY).toInt();
    keyRepeatRate = settings.value("keyRepeatRate", KEYREPEAT_RATE).toInt();

    keymap->shift->setMode(modifierHandler::toKeyMode(settings.value("modifierShiftMode", MODIFIER_SHIFT_MODE).toString()));
    keymap->ctrl->setMode(modifierHandler::toKeyMode(settings.value("modifierCtrlMode", MODIFIER_CTRL_MODE).toString()));
    keymap->alt->setMode(modifierHandler::toKeyMode(settings.value("modifierAltMode", MODIFIER_ALT_MODE).toString()));
    keymap->sym->setMode(modifierHandler::toKeyMode(settings.value("modifierSymMode", MODIFIER_SYM_MODE).toString()));

    /* In case of update, we will ignore previous settings... brutally reset them to defaults */
    settings.remove("stickyShiftEnabled");
    settings.remove("lockingShiftEnabled");
    settings.remove("stickyCtrlEnabled");
    settings.remove("lockingCtrlEnabled");
    settings.remove("stickyAltEnabled");
    settings.remove("lockingAltEnabled");
    settings.remove("stickySymEnabled");
    settings.remove("lockingSymEnabled");

    forceLandscapeOrientation = settings.value("forceLandscapeOrientation", FORCE_LANDSCAPE_ORIENTATION).toBool();
    forceBacklightOn = settings.value("forceBacklightOn", FORCE_BACKLIGHT_ON).toBool();
    turnDisplayOffWhenRemoved = settings.value("turnDisplayOffWhenRemoved", TURN_DISPLAY_OFF_WHEN_REMOVED).toBool();
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
    if (verboseMode)
        printf("shortcut %s = \"%s\"\n", qPrintable(key), qPrintable(appPath));

    if (key.startsWith("F") && (appPath.contains(".desktop") || appPath.isEmpty()))
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

    if (verboseMode)
        printf("Setting %s to %d\n", qPrintable(key), value);

    if (key == "backlightTimeout" && value >= 100 && value <= 30000)
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
            tohkbd2user->setOrientationLock(currentOrientationLock);
        }
        else if (value == 1 && keypadIsPresent)
        {
            tohkbd2user->setOrientationLock("landscape");
        }
    }
    else if (key == "turnDisplayOffWhenRemoved" && (value == 0 || value == 1))
    {
        settings.beginGroup("generalsettings");
        settings.setValue("turnDisplayOffWhenRemoved", (value == 1));
        turnDisplayOffWhenRemoved = (value == 1);
        settings.endGroup();
    }
    else if (key == "verboseMode"  && (value == 0 || value == 1))
    {
        settings.beginGroup("debug");
        settings.setValue("verboseMode", (value == 1));
        setVerboseMode(value == 1);
        settings.endGroup();
    }

    keymap->releaseStickyModifiers(true);
}

void Tohkbd::setSettingString(const QString &key, const QString &value)
{
    QSettings settings(QSettings::SystemScope, "harbour-tohkbd2", "tohkbd2");

    settings.beginGroup("generalsettings");
    settings.setValue(key, value);
    settings.endGroup();

    if(verboseMode)
        printf("setting %s to %s\n", qPrintable(key), qPrintable(value));

    if (key == "modifierShiftMode" && modifierHandler::KeyModeNames.contains(value))
        keymap->shift->setMode(modifierHandler::toKeyMode(value));

    if (key == "modifierCtrlMode" && modifierHandler::KeyModeNames.contains(value))
        keymap->ctrl->setMode(modifierHandler::toKeyMode(value));

    if (key == "modifierAltMode" && modifierHandler::KeyModeNames.contains(value))
        keymap->alt->setMode(modifierHandler::toKeyMode(value));

    if (key == "modifierSymMode" && modifierHandler::KeyModeNames.contains(value))
        keymap->sym->setMode(modifierHandler::toKeyMode(value));

}

/* Tell user daemon to show notification */
void Tohkbd::keyboardConnectedNotification(bool connected)
{
    tohkbd2user->showKeyboardConnectionNotification(connected);
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
    QDeviceInfo deviceInfo;
    QString sailfishVersion = deviceInfo.version(QDeviceInfo::Os);

    if (verboseMode)
        printf("Sailfish version %s\n", qPrintable(sailfishVersion));

    /* The version number is form 1.2.3.4 which is like IPv4 address
     * Following uses QHostAddress.toIPv4Address() to convert
     * version strings to comparable numbers */
    return (QHostAddress(sailfishVersion).toIPv4Address()
            >= QHostAddress(versionToCompare).toIPv4Address());
}

/* UinputEvPoll will emit signal if caps lock led state is seen in evdev
 */
void Tohkbd::capsLockLedState(bool state)
{
    if (state != capsLock)
    {
        capsLock = state;

        if (displayIsOn)
        {
            if (verboseMode)
                printf("caps lock led state changed to %s\n", state ? "on" : "off");

            if (capsLock)
            {
                tca8424->setLeds(LED_CAPSLOCK_ON);
            }
            else
            {
                tca8424->setLeds(LED_CAPSLOCK_OFF);
            }
        }
    }
}

/* Slot connected to userdaemon signal telling physical layout has changed
 */
void Tohkbd::handlePhysicalLayout(const QString &layout)
{
    if (verboseMode)
        printf("physcial layout changed to \"%s\"\n", qPrintable(layout));

    if (!keymap->setLayout(layout))
    {
        /* Inform the user that selected layout is not supported... */
        tohkbd2user->showUnsupportedLayoutNotification();
    }
}

/* DBUS method for forcing the keymap loading
 * if "layout" is empty, get current layout from dconf and try to reload it
 */
void Tohkbd::forceKeymapReload(const QString &layout)
{
    QString toLayout = layout.isEmpty() ? QString(tohkbd2user->getActivePhysicalLayout()) : layout;

    printf("forced keymap reload \"%s\"\n", qPrintable(toLayout));

    if (!keymap->setLayout(toLayout, true))
    {
        tohkbd2user->showUnsupportedLayoutNotification();
    }
}

void Tohkbd::setVerboseMode(bool verbose)
{
    verboseMode = verbose;

    keymap->verboseMode = verbose;
    keymap->shift->verboseMode = verbose;
    keymap->ctrl->verboseMode = verbose;
    keymap->alt->verboseMode = verbose;
    keymap->sym->verboseMode = verbose;
}

/*
 * Gets current display state from mce
 * returns false if off, else true
 */
bool Tohkbd::getCurrentDisplayState()
{
    QDBusMessage m = QDBusMessage::createMethodCall(MCE_SERVICE, MCE_REQUEST_PATH, MCE_REQUEST_IF,  MCE_DISPLAY_STATUS_GET);
    QString reply = QDBusConnection::systemBus().call(m).arguments().at(0).toString();

    if (verboseMode)
        printf("Display status is \"%s\"\n", qPrintable(reply));

    return (reply.compare(MCE_DISPLAY_OFF_STRING) != 0);
}
