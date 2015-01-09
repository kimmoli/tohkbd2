#include "eepromdriver.h"

eepromDriver::eepromDriver(unsigned char address)
{
    eepromAddress = address;
}

QByteArray eepromDriver::readData(int address, int count)
{
    char data[1];
    data[0] = (char)address;

    if (!writeBytes(eepromAddress, data, 1))
        return QByteArray();

    return readBytes(eepromAddress, count);
}

bool eepromDriver::writeData(int address, QByteArray data)
{
    for (int i=0 ; i<data.length() ; i=i+8)
    {
        char tmp[9] = {};
        int k = 1;

        for (int j=i ; j<data.length() ; j++)
        {
            tmp[k++] = data.at(j);
        }
        tmp[0] = (char)(address + i);

        if (!writeBytes(eepromAddress, tmp, k))
            return false;
    }
    return true;
}
