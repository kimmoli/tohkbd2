/*
 * (C) 2014 Kimmo Lindholm <kimmo.lindholm@gmail.com> Kimmoli
 *
 * Toh Worker code for interrupt service
 *
 */

#include <poll.h>
#include <unistd.h>
#include <QEventLoop>
#include <QThread>

#include "worker.h"


Worker::Worker(QObject *parent) :
    QObject(parent)
{
    _working = false;
    _abort = false;
    _gpio_fd = -1;
}

void Worker::requestWork(int gpio_fd)
{
    mutex.lock();
    _working = true;
    _gpio_fd = gpio_fd;
    _abort = false;

    mutex.unlock();

    emit workRequested();
}

void Worker::abort()
{
    mutex.lock();
    if (_working)
    {
        _abort = true;
    }
    mutex.unlock();
}

void Worker::doWork()
{
    struct pollfd fdset[1];
    unsigned char buf[20];

    for (;;)
    {

        // Checks if the process should be aborted
        mutex.lock();
        bool abort = _abort;
        mutex.unlock();

        if (abort)
        {
            break;
        }

        memset((void*)fdset, 0, sizeof(fdset));

        fdset[0].fd = _gpio_fd;
        fdset[0].events = POLLPRI;
        poll(fdset, 1, 1800);

        if (fdset[0].revents & POLLPRI)
        {
            ssize_t res = read(fdset[0].fd, buf, sizeof(buf));
            (void) res;
            emit gpioInterruptCaptured();
        }

    }

    mutex.lock();
    _working = false;
    mutex.unlock();

    emit finished();
}
