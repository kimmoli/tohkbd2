/*
 * (C) 2014 Kimmo Lindholm <kimmo.lindholm@gmail.com> Kimmoli
 *
 * uinput functions
 *
 *
 *
 *
 */

#include "uinputif.h"

#include <linux/input.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

static const char conf_devname[] = "tohkbd";
static const char specialKeys[] = { KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
                                    KEY_KPPLUS, KEY_HOME, KEY_END,
                                    KEY_PAGEDOWN, KEY_PAGEUP, 0 };

int UinputIf::fd = -1;

UinputIf::UinputIf(QObject *parent) :
    QObject(parent)
{
}


/*
 * open and configure uinput device for tohkbd
 *
 */

int UinputIf::openUinputDevice()
{
    int i;
    struct uinput_user_dev uidev;


    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0)
    {
        printf("uinput: error: open\n");
        return false;
    }

    if (ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0)
    {
        printf("uinput: error: ioctl UI_SET_EVBIT EV_KEY\n");
        return false;
    }

    if (ioctl(fd, UI_SET_EVBIT, EV_SYN) < 0)
    {
        printf("uinput: error: ioctl UI_SET_EVBIT EV_SYN\n");
        return false;
    }

    /* Enable most of the keys */
    for (i = KEY_ESC; i < KEY_NUMLOCK; i++)
    {
        if (ioctl(fd, UI_SET_KEYBIT, i) < 0)
        {
            printf("uinput: error: ioctl UI_SET_KEYBIT\n");
            return false;
        }
    }

    for (i=0; specialKeys[i] != 0; i++)
    {
        if (ioctl(fd, UI_SET_KEYBIT, specialKeys[i]) < 0)
        {
            printf("uinput: error: ioctl UI_SET_KEYBIT\n");
            return false;
        }
    }

    //printf("uinput: /dev/uinput opened succesfully.\n");

    memset(&uidev, 0, sizeof(uidev));
    strncpy(uidev.name, conf_devname, UINPUT_MAX_NAME_SIZE);
    uidev.id.bustype = BUS_VIRTUAL;
    uidev.id.vendor  = 0x4b4c;
    uidev.id.product = 0x4f48;
    uidev.id.version = 1;

    if (write(fd, &uidev, sizeof(uidev)) < 0)
    {
        printf("uinput: error: write uidev\n");
        return false;
    }

    if (ioctl(fd, UI_DEV_CREATE) < 0)
    {
        printf("uinput: error: ioctl UI_DEV_CREATE\n");
        return false;
    }

    usleep(100000); /* wait magic 100ms here*/

    return true;
}

/*
 * Send EV_SYN event
 *
 */

int UinputIf::synUinputDevice()
{
    struct input_event     ev;

    usleep(40000);
    memset(&ev, 0, sizeof(struct input_event));
    gettimeofday(&ev.time, NULL);
    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    if (write(fd, &ev, sizeof(struct input_event)) < 0)
    {
        printf("uinput: error: EV_SYN write\n");
        return false;
    }
    return true;
}


/*
 * submits key presses
 * code = KEY_0
 * val = 1 down, 0 up
 *
 */

int UinputIf::sendUinputKeyPress(unsigned int code, int val)
{
    struct input_event     ev;

    usleep(25000); /* sorcery */
    memset(&ev, 0, sizeof(struct input_event));
    gettimeofday(&ev.time, NULL);
    ev.type = EV_KEY;
    ev.code = code;
    ev.value = val;
    if (write(fd, &ev, sizeof(struct input_event)) < 0)
    {
        printf("uinput: error: EV_KEY write\n");
        return false;
    }

    return true;
}


/*
 * destroys and closes the opened uinput device
 *
 */

int UinputIf::closeUinputDevice()
{
    usleep(100000);

    if (ioctl(fd, UI_DEV_DESTROY) < 0)
    {
        printf("uinput: error: ioctl\n");
        return false;
    }

    close(fd);
    return true;
}
