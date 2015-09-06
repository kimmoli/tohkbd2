/*
 * (C) 2014 Kimmo Lindholm <kimmo.lindholm@gmail.com> Kimmoli
 *
 */

#ifndef TOHKEYBOARD_H
#define TOHKEYBOARD_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>

#include <QTime>
#include <QTimer>
#include <QThread>
#include "worker.h"
#include <QList>
#include <QPair>
#include <QHash>

#include "uinputif.h"
#include "uinputevpoll.h"

#include "tca8424driver.h"
#include "keymapping.h"
#include "../dbus/src/userdaemonInterface.h"
#include "../dbus/src/settingsuiInterface.h"

#define SERVICE_NAME "com.kimmoli.tohkbd2"
#define EVDEV_OFFSET (8)

/* main class */

class QDBusInterface;
class Tohkbd: public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", SERVICE_NAME)

public:
    explicit Tohkbd(QObject *parent = 0);
    virtual ~Tohkbd();

    void registerDBus();

public slots:
    /* dbus signal handler slots */
    void handleDisplayStatus(const QDBusMessage& msg);

    /* keymap handler slots */
    void handleShiftChanged();
    void handleCtrlChanged();
    void handleAltChanged();
    void handleSymChanged();
    void handleKeyPressed(QList< QPair<int, int> > keyCode);
    void handleKeyReleased();
    void handlePhysicalLayout(const QString &layout);
    void toggleCapsLock();
    void capsLockLedState(bool state);

    /* timer timeouts */
    void backlightTimerTimeout();
    void presenceTimerTimeout();
    void repeatTimerTimeout();

    /* Interrupt */
    void handleGpioInterrupt();

    /* DBUS methods */
    Q_NOREPLY void fakeInputReport(const QByteArray &data);
    QString getVersion();
    Q_NOREPLY void quit();
    Q_NOREPLY void setShortcut(const QString &key, const QString &appPath);
    Q_NOREPLY void setShortcutsToDefault();
    Q_NOREPLY void setSettingInt(const QString &key, const int &value);
    Q_NOREPLY void setSettingString(const QString &key, const QString &value);
    Q_NOREPLY void forceKeymapReload(const QString &layout);

signals:

    void keyboardConnectedChanged(bool);

private:

    QString readOneLineFromFile(const QString &fileName);
    void checkDoWeNeedBacklight();
    QList<unsigned int> readEepromConfig();
    void changeActiveLayout(bool justGetIt = false);
    void changeOrientationLock(bool justGetIt = false);
    bool setVddState(bool state);
    bool setInterruptEnable(bool);
    void emitKeypadSlideEvent(bool openKeypad);
    bool checkKeypadPresence();
    void reloadSettings();
    void saveActiveLayout();
    void saveOrientation();
    void keyboardConnectedNotification(bool connected);
    void checkEEPROM();
    bool tohcoreBind(bool bind);
    void controlLeds(bool restore);
    bool checkSailfishVersion(QString versionToCompare);
    void setVerboseMode(bool verbose);

    int gpio_fd;

    QThread *thread;
    Worker *worker;
    UinputIf *uinputif;
    UinputEvPoll *uinputevpoll;
    QThread *evpollThread;
    tca8424driver *tca8424;
    keymapping *keymap;

    int backlightLuxThreshold;
    int keyRepeatDelay;
    int keyRepeatRate;
    bool forceBacklightOn;

    QMutex mutex;

    QTimer *backlightTimer;
    QTimer *presenceTimer;
    QTimer *repeatTimer;

    QString currentActiveLayout;
    QString currentOrientationLock;

    QList< QPair<int, int> > lastKeyCode;
    QHash<int, QString> applicationShortcuts;

    bool keypadIsPresent;
    bool vkbLayoutIsTohkbd;
    bool dbusRegistered;
    bool stickyCtrl;
    bool displayIsOn;
    bool vddEnabled;
    bool interruptsEnabled;
    bool keyIsPressed;
    bool keyRepeat;
    bool backlightEnabled;
    bool slideEventEmitted;
    bool forceLandscapeOrientation;
    bool taskSwitcherVisible;
    bool selfieLedOn;
    bool capsLock;
    bool verboseMode;

    ComKimmoliTohkbd2userInterface *tohkbd2user;
    ComKimmoliTohkbd2settingsuiInterface *tohkbd2settingsui;

    static QList<int> FKEYS;

    int gpioInterruptCounter;
    QTime gpioInterruptFloodDetect;

    bool fix_CapsLock;
};



#endif // TOHKEYBOARD_H
