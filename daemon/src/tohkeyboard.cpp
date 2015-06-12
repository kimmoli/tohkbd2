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
    capsLockSeq = 0;
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
    ssNotifyReplacesId = 0;
    ssFilename = QString();

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

    reloadSettings();

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
        checkDoWeNeedBacklight();
        displayIsOn = true;
        slideEventEmitted = false;
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
            tca8424->setLeds((keymap->ctrlPressed ? LED_SYMLOCK_ON : LED_SYMLOCK_OFF) | ((capsLockSeq == 3) ? LED_CAPSLOCK_ON : LED_CAPSLOCK_OFF));
            presenceTimer->start();
            checkEEPROM();
        }

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
    bool processAllKeys = true;

    presenceTimer->start();

    if (!displayIsOn && !slideEventEmitted)
    {
        emitKeypadSlideEvent(true);
        /* slideEventEmitted is used to limit events to just one
         * emitted once when key pressed while display is off
         * it is reset when display turns on. */
        slideEventEmitted = true;
    }

    if ((capsLockSeq == 1 || capsLockSeq == 2)) /* Abort caps-lock if other key pressed */
        capsLockSeq = 0;

    checkDoWeNeedBacklight();

    /* alt+ESC is the task-switcher */
    if (keymap->altPressed && keyCode.at(0).first == KEY_ESC)
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
        return;
    }

    /* Alt-p takes a screenshot */
    if (keymap->altPressed && keyCode.at(0).first == KEY_P)
    {
        screenShot();
        /* Don't process further */
        return;
    }

    /* if F1...F12 key is pressed then launch detached process */
    if (keymap->symPressed && keyCode.at(0).first >= KEY_1 && keyCode.at(0).first <= KEY_EQUAL)
    {
        QString cmd = applicationShortcuts[keyCode.at(0).first];

        if (!cmd.isEmpty())
        {
            printf("Requesting user daemon to start %s\n", qPrintable(cmd));

            QList<QVariant> args;
            args.append(cmd);
            tohkbd2user->callWithArgumentList(QDBus::AutoDetect, "launchApplication", args);

            /* Don't process further */
            return;
        }
    }

    if (keymap->symPressed && keyCode.at(0).first == KEY_UP)
    {
        uinputif->sendUinputKeyPress(KEY_PAGEUP, 1);
        QThread::msleep(KEYREPEAT_RATE);
        uinputif->sendUinputKeyPress(KEY_PAGEUP, 0);
        processAllKeys = false;
    }

    if (keymap->symPressed && keyCode.at(0).first == KEY_DOWN)
    {
        uinputif->sendUinputKeyPress(KEY_PAGEDOWN, 1);
        QThread::msleep(KEYREPEAT_RATE);
        uinputif->sendUinputKeyPress(KEY_PAGEDOWN, 0);
        processAllKeys = false;
    }

    if (processAllKeys)
    {
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
            if ((keyCode.at(i).second & FORCE_CTRL) || keymap->ctrlPressed)
                uinputif->sendUinputKeyPress(KEY_LEFTCTRL, 1);

            /* Mimic key pressing */
            uinputif->sendUinputKeyPress(keyCode.at(i).first, 1);
            QThread::msleep(KEYREPEAT_RATE);
            uinputif->sendUinputKeyPress(keyCode.at(i).first, 0);

            if ((keyCode.at(i).second & FORCE_CTRL) || keymap->ctrlPressed)
                uinputif->sendUinputKeyPress(KEY_LEFTCTRL, 0);
            if ((keyCode.at(i).second & FORCE_ALT) || keymap->altPressed)
                uinputif->sendUinputKeyPress(KEY_LEFTALT, 0);
            if ((keyCode.at(i).second & FORCE_SHIFT) || keymap->shiftPressed)
                uinputif->sendUinputKeyPress(KEY_LEFTSHIFT, 0);
            if ((keyCode.at(i).second & FORCE_RIGHTALT) || keymap->symPressed)
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

    if (keymap->stickyCtrlEnabled)
    {
        tca8424->setLeds(keymap->ctrlPressed ? LED_SYMLOCK_ON : LED_SYMLOCK_OFF); /* TODO: Fix correct led when such is in HW */
    }
}

void Tohkbd::handleAltChanged()
{
    if ((capsLockSeq == 1 || capsLockSeq == 2)) /* Abort caps-lock if other key pressed */
        capsLockSeq = 0;

    printf("alt changed %s\n", keymap->altPressed ? "down" : "up");

    if (!keymap->altPressed && taskSwitcherVisible)
    {
        /* hide taskswitcher when alt is released
         * this will also activate selected application */
        tohkbd2user->call(QDBus::AutoDetect, "hideTaskSwitcher");
        taskSwitcherVisible = false;
    }
}

void Tohkbd::handleSymChanged()
{
    if ((capsLockSeq == 1 || capsLockSeq == 2)) /* Abort caps-lock if other key pressed */
        capsLockSeq = 0;

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
    if (backlightEnabled)
    {
        if (!backlightTimer->isActive())
        {
            if (readOneLineFromFile("/sys/devices/virtual/input/input11/als_lux").toInt() < backlightLuxThreshold)
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
}

/* Backlight timer timeout, will turn backlight off
 */
void Tohkbd::backlightTimerTimeout()
{
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

    applicationShortcuts[KEY_1] = settings.value(QString("KEY_F1"), SHORTCUT_KEY_F1).toString();
    applicationShortcuts[KEY_2] = settings.value(QString("KEY_F2"), SHORTCUT_KEY_F2).toString();
    applicationShortcuts[KEY_3] = settings.value(QString("KEY_F3"), SHORTCUT_KEY_F3).toString();
    applicationShortcuts[KEY_4] = settings.value(QString("KEY_F4"), SHORTCUT_KEY_F4).toString();
    applicationShortcuts[KEY_5] = settings.value(QString("KEY_F5"), SHORTCUT_KEY_F5).toString();
    applicationShortcuts[KEY_6] = settings.value(QString("KEY_F6"), SHORTCUT_KEY_F6).toString();
    applicationShortcuts[KEY_7] = settings.value(QString("KEY_F7"), SHORTCUT_KEY_F7).toString();
    applicationShortcuts[KEY_8] = settings.value(QString("KEY_F8"), SHORTCUT_KEY_F8).toString();
    applicationShortcuts[KEY_9] = settings.value(QString("KEY_F9"), SHORTCUT_KEY_F9).toString();
    applicationShortcuts[KEY_0] = settings.value(QString("KEY_F10"), SHORTCUT_KEY_F10).toString();
    applicationShortcuts[KEY_MINUS] = settings.value(QString("KEY_F11"), SHORTCUT_KEY_F11).toString();
    applicationShortcuts[KEY_EQUAL] = settings.value(QString("KEY_F12"), SHORTCUT_KEY_F12).toString();

    for (int i = KEY_1 ; i<=KEY_EQUAL ; i++)
    {
        printf("app shortcut F%d : %s\n", ((i-KEY_1)+1), qPrintable(applicationShortcuts[i]));
        /* Write them back, as we need default values there in settings app */
        settings.setValue(QString("KEY_F%1").arg(((i-KEY_1)+1)), applicationShortcuts[i]);
    }
    settings.endGroup();

    settings.beginGroup("generalsettings");
    backlightTimer->setInterval(settings.value("backlightTimeout", BACKLIGHT_TIMEOUT).toInt());
    backlightLuxThreshold = settings.value("backlightLuxThreshold", BACKLIGHT_LUXTHRESHOLD).toInt();
    backlightEnabled = settings.value("backlightEnabled", BACKLIGHT_ENABLED).toBool();
    keyRepeatDelay = settings.value("keyRepeatDelay", KEYREPEAT_DELAY).toInt();
    keyRepeatRate = settings.value("keyRepeatRate", KEYREPEAT_RATE).toInt();
    keymap->stickyCtrlEnabled = settings.value("stickyCtrlEnabled", STICKY_CTRL_ENABLED).toBool();
    keymap->stickyAltEnabled = settings.value("stickyAltEnabled", STICKY_ALT_ENABLED).toBool();
    keymap->stickySymEnabled = settings.value("stickySymEnabled", STICKY_SYM_ENABLED).toBool();
    forceLandscapeOrientation = settings.value("forceLandscapeOrientation", FORCE_LANDSCAPE_ORIENTATION).toBool();
    settings.endGroup();

    settings.beginGroup("orientation");
    currentOrientationLock = settings.value("originalOrientation", QString()).toString();
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

        for (int i = KEY_1 ; i<=KEY_EQUAL ; i++)
        {
            applicationShortcuts[i] = settings.value(QString("KEY_F%1").arg((i-KEY_1)+1), applicationShortcuts[i]).toString();
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

    applicationShortcuts[KEY_1] = SHORTCUT_KEY_F1;
    applicationShortcuts[KEY_2] = SHORTCUT_KEY_F2;
    applicationShortcuts[KEY_3] = SHORTCUT_KEY_F3;
    applicationShortcuts[KEY_4] = SHORTCUT_KEY_F4;
    applicationShortcuts[KEY_5] = SHORTCUT_KEY_F5;
    applicationShortcuts[KEY_6] = SHORTCUT_KEY_F6;
    applicationShortcuts[KEY_7] = SHORTCUT_KEY_F7;
    applicationShortcuts[KEY_8] = SHORTCUT_KEY_F8;
    applicationShortcuts[KEY_9] = SHORTCUT_KEY_F9;
    applicationShortcuts[KEY_0] = SHORTCUT_KEY_F10;
    applicationShortcuts[KEY_MINUS] = SHORTCUT_KEY_F11;
    applicationShortcuts[KEY_EQUAL] = SHORTCUT_KEY_F12;

    for (int i = KEY_1 ; i<=KEY_EQUAL ; i++)
    {
        settings.setValue(QString("KEY_F%1").arg((i-KEY_1)+1), applicationShortcuts[i]);
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
        backlightTimer->setInterval(value);
        settings.beginGroup("generalsettings");
        settings.setValue("backlightTimeout", value);
        settings.endGroup();
    }
    else if (key == "backlightLuxThreshold" && value >= 1 && value <= 50)
    {
        backlightLuxThreshold = value;
        settings.beginGroup("generalsettings");
        settings.setValue("backlightLuxThreshold", value);
        settings.endGroup();
    }
    else if (key == "keyRepeatDelay" && value >= 50 && value <= 500)
    {
        keyRepeatDelay = value;
        settings.beginGroup("generalsettings");
        settings.setValue("keyRepeatDelay", value);
        settings.endGroup();
    }
    else if (key == "keyRepeatRate" && value >= 25 && value <= 100)
    {
        keyRepeatRate = value;
        settings.beginGroup("generalsettings");
        settings.setValue("keyRepeatRate", value);
        settings.endGroup();
    }
    else if (key == "backlightEnabled" && (value == 0 || value == 1))
    {
        backlightEnabled = (value == 1);
        settings.beginGroup("generalsettings");
        settings.setValue("backlightEnabled", (value == 1));
        settings.endGroup();
    }
    else if (key == "forceLandscapeOrientation" && (value == 0 || value == 1))
    {
        forceLandscapeOrientation = (value == 1);
        settings.beginGroup("generalsettings");
        settings.setValue("forceLandscapeOrientation", (value == 1));
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


/* Screeshot */

void Tohkbd::screenShot()
{
    QDate ssDate = QDate::currentDate();
    QTime ssTime = QTime::currentTime();

    ssFilename = QString("%8/ss%1%2%3-%4%5%6-%7.png")
                    .arg((int) ssDate.day(),    2, 10, QLatin1Char('0'))
                    .arg((int) ssDate.month(),  2, 10, QLatin1Char('0'))
                    .arg((int) ssDate.year(),   2, 10, QLatin1Char('0'))
                    .arg((int) ssTime.hour(),   2, 10, QLatin1Char('0'))
                    .arg((int) ssTime.minute(), 2, 10, QLatin1Char('0'))
                    .arg((int) ssTime.second(), 2, 10, QLatin1Char('0'))
                    .arg((int) ssTime.msec(),   3, 10, QLatin1Char('0'))
                    .arg("/home/nemo/Pictures");


    QDBusMessage m = QDBusMessage::createMethodCall("org.nemomobile.lipstick",
                                                    "/org/nemomobile/lipstick/screenshot",
                                                    "",
                                                    "saveScreenshot" );

    QList<QVariant> args;
    args.append(ssFilename);
    m.setArguments(args);

    if (QDBusConnection::sessionBus().send(m))
        printf("Screenshot success to %s\n", qPrintable(ssFilename));
    else
        printf("Screenshot failed\n");

    notificationSend("Screenshot saved", ssFilename);
}


void Tohkbd::notificationSend(QString summary, QString body)
{

    QDBusInterface notifyCall("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "");

    QVariantHash hints;
    hints.insert("x-nemo-preview-summary", summary);

    QList<QVariant> args;
    args.append("tohkbd2");
    args.append(ssNotifyReplacesId);
    args.append("icon-m-notifications");
    args.append(summary);
    args.append(body);
    args.append((QStringList() << "default" << ""));
    args.append(hints);
    args.append(-1);

    QDBusMessage notifyCallReply = notifyCall.callWithArgumentList(QDBus::AutoDetect, "Notify", args);

    QList<QVariant> outArgs = notifyCallReply.arguments();

    ssNotifyReplacesId = outArgs.at(0).toInt();

    printf("Notification sent, got id %d\n", ssNotifyReplacesId);
}

void Tohkbd::handleNotificationActionInvoked(const QDBusMessage& msg)
{

    QList<QVariant> outArgs = msg.arguments();

    unsigned int notificationId = outArgs.at(0).toInt();

    /* Manage the screenshot notification id action. */
    if (notificationId == ssNotifyReplacesId)
    {
        printf("Screenshot notification id %d Action invoked - opening image %s\n", notificationId, qPrintable(ssFilename));

        QDBusMessage m = QDBusMessage::createMethodCall("com.jolla.gallery",
                                                        "/com/jolla/gallery/ui",
                                                        "com.jolla.gallery.ui",
                                                        "showImages" );

        QList<QVariant> args;
        args.append((QStringList() << ssFilename));
        m.setArguments(args);

        if (!QDBusConnection::sessionBus().send(m))
            printf("Failed to invoke gallery to show %s\n", qPrintable(ssFilename));
    }
}
