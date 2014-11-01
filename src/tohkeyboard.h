/*
 * (C) 2014 Kimmo Lindholm <kimmo.lindholm@gmail.com> Kimmoli
 *
 */

#ifndef TOHKEYBOARD_H
#define TOHKEYBOARD_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>

#include <QTime>
#include <QThread>
#include "worker.h"

#include "uinputif.h"


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
    void handleDisplayStatus(const QDBusMessage& msg);


private:
    QThread *thread;
    Worker *worker;
    UinputIf *uinputif;

    static bool vddEnabled;
    static bool interruptsEnabled;

    static int capsLockSeq;

    QMutex mutex;

    int gpio_fd;
};



#endif // TOHKEYBOARD_H
