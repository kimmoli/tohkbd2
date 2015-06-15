#include "tca8424driver.h"
#include <QThread>

tca8424driver::tca8424driver(unsigned char address)
{
    tca8424address = address;

    init();
}

void tca8424driver::init()
{
    ledState = 0;

    reset();
    QThread::msleep(100);
    setLeds(LED_CAPSLOCK_OFF | LED_SYMLOCK_OFF | LED_BACKLIGHT_OFF);

    printf("tca8424 at %02x initialized\n", tca8424address);
}

bool tca8424driver::reset()
{
    /* HID Over I2C protocol v1.0 chapter 7.2.1 Host initiated Reset (HIR)
     * Command register 0x0600
     * High byte:
     *  Reserved = 0x0
     *  opcode = 0x1
     * Low byte:
     * Reserved = 0x00
     */
    char buf[4] = {0x00, 0x06, 0x00, 0x01};

    bool ret = writeBytes(tca8424address, buf, sizeof(buf));

    if (!ret)
    {
        printf("Error: reset() failed\n");
    }

    return ret;

}

void tca8424driver::setLeds(int value)
{
    /* TOHKBD Rev 2 Hardware led assignments
     * LED0 = Extra
     * LED1 = Caps-lock
     * LED2 = Sym-lock
     * LED3 = Backlight
     */

    ledState = ((ledState & ~((value >> 8) & 0x0F)) | (value & 0x0F));

    char buf[5] = {0x00, 0x05, 0x01, 0x00, ledState};

    if (!writeBytes(tca8424address, buf, sizeof(buf)))
        printf("Error: leds(%02x %02x) failed\n", value, ledState);
}

QByteArray tca8424driver::readInputReport()
{
    QByteArray ret = QByteArray();

    ret =  readBytes(tca8424address, 11);

    if (ret.isEmpty())
        printf("Error: readInputReport() failed\n");

    return ret;
}

tca8424driver::PresenceResult tca8424driver::testComms()
{
    QByteArray ret = QByteArray();

    ret = readBytes(tca8424address, 2);

    if (ret.isEmpty())
    {
        /* Retry once after 100 ms */
        QThread::msleep(100);
        ret = readBytes(tca8424address, 2);

        if (ret.isEmpty())
            return tca8424driver::DetectFail;
    }

    if ((ret.at(0) == 0) && (ret.at(1) == 0))
        return tca8424driver::NoKeyPressedSinceReset;

    return tca8424driver::DetectSuccess;
}
