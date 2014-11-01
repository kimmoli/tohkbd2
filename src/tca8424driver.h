#ifndef TCA8424DRIVER_H
#define TCA8424DRIVER_H
#include "driverBase.h"


#define LED_CAPSLOCK_ON   0x0101
#define LED_CAPSLOCK_OFF  0x0100
#define LED_SYMLOCK_ON    0x0202
#define LED_SYMLOCK_OFF   0x0200
#define LED_BACKLIGHT_ON  0x0404
#define LED_BACKLIGHT_OFF 0x0400


class tca8424driver : public DriverBase
{
    Q_OBJECT
public:
    explicit tca8424driver(unsigned char address);

    bool reset();
    void setLeds(int value);
    QByteArray readInputReport();

signals:

public slots:

private:
    void init();
    unsigned char tca8424address;
    unsigned char ledState;

};

#endif // TCA8424DRIVER_H
