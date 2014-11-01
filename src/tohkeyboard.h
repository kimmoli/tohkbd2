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

#include "uinputif.h"

#include "tca8424driver.h"
#include "keymapping.h"


/* main class */

class Tohkbd: public QObject
{
    Q_OBJECT

public:
    Tohkbd();

    ~Tohkbd()
    {
        uinputif->closeUinputDevice();

        worker->abort();
        thread->wait();
        delete thread;
        delete worker;
    }

public slots:
    bool setVddState(bool state);

    /* interrupts */
    bool setInterruptEnable(bool);
    void handleGpioInterrupt();

    /* dbus signal handler slots */
    void handleDisplayStatus(const QDBusMessage& msg);

    /* keymap handler slots */
    void handleShiftChanged();
    void handleCtrlChanged();
    void handleAltChanged();
    void handleSymChanged();
    void handleKeyPressed(int keyCode, bool forceShift);

    void backlightTimerTimeout();


private:

    QString readOneLineFromFile(QString name);
    void checkDoWeNeedBacklight();
    QList<unsigned int> readEepromConfig();

    QThread *thread;
    Worker *worker;
    UinputIf *uinputif;
    tca8424driver *tca8424;
    keymapping *keymap;

    bool vddEnabled;
    bool interruptsEnabled;

    int capsLockSeq;

    QMutex mutex;

    int gpio_fd;

    bool stickyCtrl;

    QTimer *backlightTimer;

};



#endif // TOHKEYBOARD_H
