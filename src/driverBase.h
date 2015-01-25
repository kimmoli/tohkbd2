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

#ifndef DRIVERBASE_H
#define DRIVERBASE_H
#include <QString>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <QDebug>
#include <QObject>
#include <QByteArray>

class DriverBase : public QObject {

public:

    // write given bytes to specified address
    bool writeBytes(unsigned char address, char bytes[], int length);

    // read specified amount of bytes
    QByteArray readBytes(unsigned char address, int howManyBytesToRead);

    // first write the register from which to read and then read
    QByteArray writeThenRead(unsigned char address,
                             char registerToRead,
                             int howManyBytesToRead);

private:
    int openDeviceFile(const char *name);
    bool setSlaveAddress(int file, unsigned char address);
    bool i2cWrite(int file, char buffer[], int buffer_length);
    bool i2cRead(int file, char buffer[], int howManyBytesToRead);
};

#endif // DRIVERBASE_H
