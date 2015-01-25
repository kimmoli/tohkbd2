/*
 * (C) 2014 Kimmo Lindholm <kimmo.lindholm@gmail.com> Kimmoli
 *
 * toh generic daemon, TOH Low level control funtions
 *
 *
 *
 *
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "toh.h"


int controlVdd(int state)
{
    int fd;
    ssize_t res;

    fd = open("/sys/devices/platform/reg-userspace-consumer.0/state", O_WRONLY);
    if (fd < 0)
    {
        printf("failed to open /sys/devices/platform/reg-userspace-consumer.0/state\n");
        return -1;
    }

    res = write (fd, state ? "1" : "0", 1);
    close(fd);
    if (res != 1)
    {
        printf("failed to write \"%s\" to /sys/devices/platform/reg-userspace-consumer.0/state\n",
               state ? "1" : "0");
        return -1;
    }

    return 0;
}

int releaseTohInterrupt(int fdGpio)
{
    int fd;
    ssize_t res;

    if (fdGpio >= 0)
        close(fdGpio);

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd < 0)
    {
        printf("failed to open /sys/class/gpio/unexport\n");
        return -1;
    }

    res = write (fd, GPIO_INT, strlen(GPIO_INT));
    close(fd);
    if (res != strlen(GPIO_INT))
    {
        printf("failed to write \"" GPIO_INT "\" to /sys/class/gpio/unexport\n");
        return -1;
    }

    return 0;
}


int getTohInterrupt()
{
    int fd;
    ssize_t res;

    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0)
    {
        printf("failed to open /sys/class/gpio/export\n");
        return -1;
    }

    res = write (fd, GPIO_INT, strlen(GPIO_INT));
    close(fd);

    if (res != strlen(GPIO_INT))
    {
        printf("failed to write \"" GPIO_INT "\" to /sys/class/gpio/export\n");
        return -1;
    }

    fd = open("/sys/class/gpio/gpio" GPIO_INT "/edge", O_WRONLY);
    if (fd < 0)
    {
        printf("failed to open /sys/class/gpio/gpio" GPIO_INT "/edge\n");
        return -1;
    }

    res = write (fd, GPIO_INT_EDGE, strlen(GPIO_INT_EDGE));
    close(fd);
    if (res != strlen(GPIO_INT_EDGE))
    {
        printf("failed to write \"" GPIO_INT_EDGE "\" to /sys/class/gpio/gpio" GPIO_INT "/edge\n");
        return -1;
    }

    fd = open("/sys/class/gpio/gpio" GPIO_INT "/value", O_RDONLY | O_NONBLOCK);
    if (fd < 0)
    {
        printf("failed to open /sys/class/gpio/gpio" GPIO_INT "/value\n");
    }
    return fd;
}
