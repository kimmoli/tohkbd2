/*
The MIT License (MIT)

Copyright (c) 2014 Olli-Pekka Heinisuo

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "driverBase.h"

int DriverBase::openDeviceFile(const char* name)
{
    int file = open(name, O_RDWR);
    if (file < 0) {
        qDebug() << "Device file open failed.";
        return -1;
    }
    return file;
}

bool DriverBase::setSlaveAddress(int file, unsigned char address)
{
    if (ioctl(file, I2C_SLAVE, address) < 0) {
        qDebug() << "ioctl failed.";
        return false;
    }
    return true;
}

bool DriverBase::i2cWrite(int file, char buffer[], int buffer_length)
{
    if (write(file, buffer, buffer_length) != buffer_length) {
        close(file);
        qDebug() << "Write failed.";
        return false;
    }
    // qDebug() << "Write successfull.";
    return true;
}

bool DriverBase::i2cRead(int file, char buffer[], int howManyBytesToRead)
{
    if (read(file, buffer, howManyBytesToRead) != howManyBytesToRead) {
       close(file);
       qDebug() << "Read failed.";
       return false;
    }
    // qDebug() << "Read successfull.";
    return true;
}

bool DriverBase::writeBytes(unsigned char address, char bytes[], int length)
{
    // try to open the device
    int file = openDeviceFile("/dev/i2c-1");

    if (file != -1) {
        // set slave address
        if (setSlaveAddress(file, address)) {
            // try to perform the actual write
            if (i2cWrite(file, bytes, length)) {
                close(file);
                return true;
            }
        }
    }
    close(file);
    return false;
}

QByteArray DriverBase::readBytes(unsigned char address, int howManyBytesToRead)
{
    QByteArray result;
    char buffer[256];

    if (howManyBytesToRead > 255)
    {
        qDebug() << "Reading too many bytes.";
        return QByteArray();
    }

    // try to open the device
    int file = openDeviceFile("/dev/i2c-1");

    if (file != -1) {
        // set slave address
        if (setSlaveAddress(file, address)) {
            // read from the register
            if(i2cRead(file, buffer, howManyBytesToRead)) {
                // convert to QByteArray
                close(file);
                for(int i = 0; i < howManyBytesToRead; ++i) {
                    result.append(buffer[i]);
                }
                return result;
            }
        }
    }
    // return empty QList in case of failure
    return result;
}

QByteArray DriverBase::writeThenRead(unsigned char address,
                                     char registerToRead,
                                     int howManyBytesToRead) {

    QByteArray result;
    char reg[1];
    reg[0] = registerToRead;

    if (writeBytes(address, reg, 1)) {
        result = readBytes(address, howManyBytesToRead);
    }
    return result;
}


