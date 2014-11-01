/*
 * (C) 2014 Kimmo Lindholm <kimmo.lindholm@gmail.com> Kimmoli
 *
 */

#ifndef UINPUTIF_H
#define UINPUTIF_H

#include <QObject>

class UinputIf : public QObject
{
    Q_OBJECT
public:
    explicit UinputIf(QObject *parent = 0);

    int openUinputDevice();
    int closeUinputDevice();
    int synUinputDevice();
    int sendUinputKeyPress(unsigned int code, int val);

signals:

public slots:

private:

    static int fd;

};

#endif // UINPUTIF_H
