#ifndef EEPROMDRIVER_H
#define EEPROMDRIVER_H
#include "driverBase.h"
#include <QByteArray>

class eepromDriver : public DriverBase
{
    Q_OBJECT
public:
    explicit eepromDriver(unsigned char address);

    QByteArray readData(int address, int count);
    bool writeData(int address, QByteArray data);

signals:

public slots:

private:
    unsigned char eepromAddress;

};

#endif // EEPROMDRIVER_H
