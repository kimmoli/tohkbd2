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

#include "uinputif.h"

#include "tca8424driver.h"
#include "keymapping.h"

#define SERVICE_NAME "com.kimmoli.tohkbd2"
#define EVDEV_OFFSET (8)

#define REPEAT_DELAY (250)
#define REPEAT_RATE (13)

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

    /* timer timeouts */
    void backlightTimerTimeout();
    void presenceTimerTimeout();
    void repeatTimerTimeout();

    /* Interrupt */
    void handleGpioInterrupt();

    /* Notification */
    void showNotification(QString text);

    /* DBUS methods */
    Q_NOREPLY void fakeInputReport(const QByteArray &data);
    QString getVersion();
    Q_NOREPLY void quit();

signals:

    void keyboardConnectedChanged(bool);

private:

    QString readOneLineFromFile(const QString &fileName);
    QString readOneLineFromFile(const QString &fileName, const QString &pattern);
    void checkDoWeNeedBacklight();
    QList<unsigned int> readEepromConfig();
    void changeActiveLayout(bool justGetIt = false);
    bool setVddState(bool state);
    bool setInterruptEnable(bool);
    void emitKeypadSlideEvent(bool openKeypad);
    bool checkKeypadPresence();
    void reloadSettings();
    void writeSettings();

    int gpio_fd;

    QThread *thread;
    Worker *worker;
    UinputIf *uinputif;
    tca8424driver *tca8424;
    keymapping *keymap;

    int capsLockSeq;

    QMutex mutex;

    QTimer *backlightTimer;
    QTimer *presenceTimer;
    QTimer *repeatTimer;

    QString currentActiveLayout;

    QList< QPair<int, int> > lastKeyCode;

    bool keypadIsPresent;
    bool vkbLayoutIsTohkbd;
    bool dbusRegistered;
    bool stickyCtrl;
    bool displayIsOn;
    bool vddEnabled;
    bool interruptsEnabled;
    bool keyIsPressed;

};



#endif // TOHKEYBOARD_H
