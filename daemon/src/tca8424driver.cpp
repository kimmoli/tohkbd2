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
     * LED0 = Caps-lock
     * LED1 = Sym-lock?
     * LED2 = Backlight
     * LED3 = Not used
     */

    ledState = ((ledState & ~((value >> 8) & 0x07)) | (value & 0x07));

    /* HID Over I2C protocol v1.0 chapter 7.23 SET_REPORT
     * Command register 0x0600
     * High byte:
     *  Reserved = 0x0
     *  opcode = 0x3
     * Low byte:
     *  Report type = 0x2 (Output)
     *  Report ID = 0x0
     * Data register 0x0700
     *  Length = 0x0001
     *  Data = leds
     */
    char buf[9] = {0x00, 0x06, 0x20, 0x03, 0x00, 0x07, 0x01, 0x00, ledState};

    if (!writeBytes(tca8424address, buf, sizeof(buf)))
        printf("Error: leds(%02x %02x) failed\n", value, ledState);
}

QByteArray tca8424driver::readInputReport()
{
    /* HID Over I2C protocol v1.0 chapter 7.23 GET_REPORT
     * Command register 0x0600
     * High byte:
     *  Reserved = 0x0
     *  opcode = 0x2
     * Low byte:
     *  Report type = 0x1 (Input)
     *  Report ID = 0x1
     * Data register 0x0700
     */
    char buf[6] = {0x00, 0x06, 0x11, 0x02, 0x00, 0x07};
    QByteArray ret = QByteArray();

    if (writeBytes(tca8424address, buf, sizeof(buf)))
    {
        /* tca8424 input report is 11 bytes:
         * Length, 2 bytes, 0x000b
         * report id, 1 byte
         * modifier keys, 1 byte
         * reserved 1 byte
         * key usage ID's 6 bytes
         */
        ret =  readBytes(tca8424address, 11);
    }

    if (ret.isEmpty())
        printf("Error: readInputReport() failed\n");

    return ret;
}

bool tca8424driver::testComms()
{
    /* Read 1 byte from tca8424, register address 0x0000
     * return true if read success
     */
    char buf[2] = {0x00, 0x00};
    QByteArray ret = QByteArray();

    if (writeBytes(tca8424address, buf, sizeof(buf)))
    {
        ret =  readBytes(tca8424address, 1);
    }

    return !ret.isEmpty();
}
