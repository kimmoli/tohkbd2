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

int UinputIf::fd = -1;

static const int eventsToRegister[] = { EV_KEY, EV_SW, EV_SYN,
                                        -1 };

static const int keysToRegister[] = {  KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,KEY_HOME, KEY_END, KEY_PAGEDOWN, KEY_PAGEUP,
                                       KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
                                       KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J,
                                       KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T,
                                       KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z, KEY_COMMA, KEY_DOT, KEY_SPACE,
                                       KEY_LEFTALT, KEY_LEFTSHIFT, KEY_LEFTCTRL,
                                       KEY_RIGHTALT, KEY_RIGHTSHIFT, KEY_RIGHTCTRL,
                                       KEY_BACKSPACE, KEY_DELETE, KEY_INSERT, KEY_EQUAL, KEY_MINUS, KEY_TAB,
                                       KEY_COMPOSE, KEY_APOSTROPHE, KEY_SEMICOLON, KEY_ENTER, KEY_ESC,
                                       KEY_POWER, KEY_PLAYCD, KEY_PAUSECD, KEY_VOLUMEDOWN, KEY_VOLUMEUP,
                                       KEY_CAPSLOCK,
                                       -1 };

static const int switchesToRegister[] = { SW_LID, SW_KEYPAD_SLIDE,
                                          -1 };

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

    /* Enable selected events */
    for (i=0; eventsToRegister[i] != -1; i++)
    {
        if (ioctl(fd, UI_SET_EVBIT, eventsToRegister[i]) < 0)
        {
            printf("uinput: error: ioctl UI_SET_EVBIT %d\n", i);
            return false;
        }
    }

    /* Enable selected keys */
    for (i=0; keysToRegister[i] != -1; i++)
    {
        if (ioctl(fd, UI_SET_KEYBIT, keysToRegister[i]) < 0)
        {
            printf("uinput: error: ioctl UI_SET_KEYBIT %d\n", i);
            return false;
        }
    }

    /* Enable selected switches */
    for (i=0; switchesToRegister[i] != -1; i++)
    {
        if (ioctl(fd, UI_SET_SWBIT, switchesToRegister[i]) < 0)
        {
            printf("uinput: error: ioctl UI_SET_SWBIT %d\n", i);
            return false;
        }
    }

    memset(&uidev, 0, sizeof(uidev));
    strncpy(uidev.name, conf_devname, UINPUT_MAX_NAME_SIZE);
    uidev.id.bustype = BUS_VIRTUAL;
    uidev.id.vendor  = 0x1989;
    uidev.id.product = 0x0003;
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
 * submits switch event
 * code = SW_KEYPAD_SLIDE
 * val = 0 CLOSED, 1 OPEN
 *
 */

int UinputIf::sendUinputSwitch(unsigned int code, int val)
{
    struct input_event     ev;

    usleep(25000); /* sorcery */
    memset(&ev, 0, sizeof(struct input_event));
    gettimeofday(&ev.time, NULL);
    ev.type = EV_SW;
    ev.code = code;
    ev.value = val;
    if (write(fd, &ev, sizeof(struct input_event)) < 0)
    {
        printf("uinput: error: EV_SW write\n");
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
