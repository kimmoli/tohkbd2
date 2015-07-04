#ifndef UINPUTEVPOLL_H
#define UINPUTEVPOLL_H

#include <QObject>
#include <QMutex>

class UinputEvPoll : public QObject
{
    Q_OBJECT
public:
    explicit UinputEvPoll(QObject *parent = 0);

    void requestPolling(int fd);
    void abort();

signals:
    void capsLockLedChanged(bool state);
    void pollingRequested();
    void finished();

public slots:
    void doPoll();

private:
    bool _polling;
    bool _abort;
    int _uinputfd;

    QMutex mutex;

    void readKeyboard(int fd);

};

#endif // UINPUTEVPOLL_H
