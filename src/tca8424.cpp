/*
 * (C) 2014 Kimmo Lindholm <kimmo.lindholm@gmail.com> Kimmoli
 *
 * Routines for TCA8424
 *
 *
 *
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include "tca8424.h"


static int tca8424_read(const char *action, int fd,
                        unsigned char *buf, ssize_t len)
{
    ssize_t res = read(fd, buf, len);
    if (res != len)
    {
        printf("tca8424: %s failed, read %zd, expected %zd: %s (%d)\n",
               action, res, len, strerror(errno), errno);
        return -1;
    }
    return 0;
}

static int tca8424_write(const char *action, int fd,
                         const unsigned char *data, ssize_t len)
{
    ssize_t res = write(fd, data, len);
    if (res != len)
    {
        printf("tca8424: %s failed, wrote %zd, expected %zd: %s (%d)\n",
               action, res, len, strerror(errno), errno);
        return -1;
    }
    return 0;
}

int tca8424_reset(int fd)
{
    const unsigned char buf[4] = {0x00, 0x06, 0x00, 0x01};
    return tca8424_write("reset", fd, buf, sizeof(buf));
}

int tca8424_leds(int fd, unsigned char leds)
{
    unsigned char buf[9] = {0x00, 0x06, 0x20, 0x03, 0x00, 0x07, 0x01, 0x00, leds};
    return tca8424_write("leds", fd, buf, sizeof(buf));
}

int tca8424_initComms(unsigned char addr)
{
    int fd;

    /* open file and start ioctl */
    if ((fd = open("/dev/i2c-1", O_RDWR)) < 0)
    {
        printf("tca8424: failed to open /dev/i2c-1\n");
        return -1;
    }
    if (ioctl(fd, I2C_SLAVE, addr) < 0)
    {
        printf("tca8424: failed to ioctl(/dev/i2c-1, I2C_SLAVE, 0x%x)\n",
               (unsigned int) addr);
        close(fd);
        return -1;
    }
    return fd;
}

int tca8424_closeComms(int fd)
{
    close(fd);
    return 0;
}

int tca8424_readInputReport(int fd, unsigned char *report)
{
    const unsigned char buf[6] = {0x00, 0x06, 0x11, 0x02, 0x00, 0x07};
    memset(report, 0, 12);

    if (tca8424_write("readInputReport", fd, buf, sizeof(buf)) < 0)
        return -1;
    return tca8424_read("readInputReport", fd, report, 11);
}

int tca8424_readMemory(int fd, int start, int len, unsigned char *data)
{
    unsigned char buf[2] =
        {(unsigned char) (start & 0xff), (unsigned char) ((start>>8) & 0xff)};
    memset(data, 0, len);

    if (tca8424_write("readMemory", fd, buf, sizeof(buf)) < 0)
        return -1;
    return tca8424_read("readMemory", fd, data, len);
}

/*
 *
 * dat Horrible key-press decoder
 *
 */

const char* tca8424_processKeyMap(unsigned char *input, int *c,
                                  int *shift, int *alt, int *ctrl)
{
    unsigned char k = input[5];

    *c = 0;
    *shift = false;
    *alt = false;
    *ctrl = false;

    /* first check for shift, alt and ctrl */
    if (k == 0x9E || k == 0xBC)
    {
        *shift = true;
    }
    else if (k == 0xAE || k == 0xBE)
    {
        *alt = true;
    }
    else if (k == 0xB3)
    {
        *ctrl = true;
        return "! Ctrl";
    }

    /* if alt, use alternate key mapping */
    if (*alt)
    {
        k = input[6];
        if (k == 0xC1) { *c = KEY_TAB; return "A Tab"; }
        if (k == 0xF1) { *c = KEY_1; return "Q 1"; }
        if (k == 0xA2) { *c = KEY_COMMA; *shift = true; return "Z <"; }
        if (k == 0xC2) { *c = KEY_1; *shift = true; return "S !"; }
        if (k == 0xF2) { *c = KEY_2; return "W 2"; }
        if (k == 0xA3) { *c = KEY_DOT; *shift = true; return "X >"; }
        if (k == 0xC3) { *c = KEY_3; *shift = true; return "D #"; }
        if (k == 0xF3) { *c = KEY_3; return "E 3"; }
        if (k == 0xA4) { *c = KEY_MINUS; *shift = true; return "C _"; }
        if (k == 0xC4) { *c = KEY_4; *shift = true; return "F $"; }
        if (k == 0xF4) { *c = KEY_4; return "R 4"; }
        if (k == 0xA5) { *c = KEY_MINUS; return "V -"; }
        if (k == 0xC5) { *c = KEY_5; *shift = true; return "G %"; }
        if (k == 0xF5) { *c = KEY_5; return "T 5"; }
        if (k == 0xA6) { *c = KEY_KPPLUS; return "B +"; }
        if (k == 0xC6) { *c = KEY_EQUAL; return "H ="; }
        if (k == 0xF6) { *c = KEY_6; return "Y 6"; }
        if (k == 0xA7) { *c = KEY_APOSTROPHE; *shift = true; return "N """; }
        if (k == 0xC7) { *c = KEY_7; *shift = true; return "J &"; }
        if (k == 0xF7) { *c = KEY_7; return "U 7"; }
        if (k == 0xA8) { *c = KEY_APOSTROPHE; return "M '"; }
        if (k == 0xC8) { *c = KEY_8; *shift = true; return "K *"; }
        if (k == 0xF8) { *c = KEY_8; return "I 8"; }
        if (k == 0xC9) { *c = KEY_9; *shift = true; return "L ("; }
        if (k == 0xF9) { *c = KEY_9; return "O 9"; }
        if (k == 0xAA) { *c = KEY_SEMICOLON; *shift = true; return ". :"; }
        if (k == 0xA9) { *c = KEY_SEMICOLON; return ", ;"; }
        if (k == 0xCA) { *c = KEY_0; *shift = true; return "? )"; }
        if (k == 0xFA) { *c = KEY_0; return "P 0"; }
        if (k == 0xB4) { *c = KEY_GRAVE; *shift = true; return "@ ~"; }
        if (k == 0xB8) { *c = KEY_6; *shift = true; return "/ ^"; }
        if (k == 0xFD) { *c = KEY_HOME; return "Left Arrow"; }
        if (k == 0x9D) { *c = KEY_END; return "Right Arrow"; }
        if (k == 0xFC) { *c = KEY_PAGEUP; *shift = true; return "Up arrow"; }
        if (k == 0xBD) { *c = KEY_PAGEDOWN; *shift = true; return "Down arrow"; }
        if (k == 0xB9) { *c = KEY_ESC; return "| Escape"; }

        if (k == 0x00) return "! Released";

        return "* Unknown";
    }

    /* if shift, check the next key in map */
    if (*shift)
        k = input[6];

    if (k == 0xC1) { *c = KEY_A; return "A Tab"; }
    if (k == 0xF1) { *c = KEY_Q; return "Q 1"; }
    if (k == 0xA2) { *c = KEY_Z; return "Z <"; }
    if (k == 0xC2) { *c = KEY_S; return "S !"; }
    if (k == 0xF2) { *c = KEY_W; return "W 2"; }
    if (k == 0xA3) { *c = KEY_X; return "X >"; }
    if (k == 0xC3) { *c = KEY_D; return "D #"; }
    if (k == 0xF3) { *c = KEY_E; return "E 3"; }
    if (k == 0xA4) { *c = KEY_C; return "C _"; }
    if (k == 0xB4) { *c = KEY_2; *shift = true; return "@ ~"; }
    if (k == 0xC4) { *c = KEY_F; return "F $"; }
    if (k == 0xF4) { *c = KEY_R; return "R 4"; }
    if (k == 0xA5) { *c = KEY_V; return "V -"; }
    if (k == 0xB5) { *c = KEY_SPACE; return "Space1"; }
    if (k == 0xC5) { *c = KEY_G; return "G %"; }
    if (k == 0xF5) { *c = KEY_T; return "T 5"; }
    if (k == 0xA6) { *c = KEY_B; return "B +"; }
    if (k == 0xB6) { *c = KEY_SPACE; return "Space2"; }
    if (k == 0xC6) { *c = KEY_H; return "H ="; }
    if (k == 0xF6) { *c = KEY_Y; return "Y 6"; }
    if (k == 0xA7) { *c = KEY_N; return "N """; }
    if (k == 0xB7) { *c = KEY_SPACE; return "Space3 Sym"; }
    if (k == 0xC7) { *c = KEY_J; return "J &"; }
    if (k == 0xF7) { *c = KEY_U; return "U 7"; }
    if (k == 0xA8) { *c = KEY_M; return "M '"; }
    if (k == 0xB8) { *c = KEY_SLASH; return "/ ^"; }
    if (k == 0xC8) { *c = KEY_K; return "K *"; }
    if (k == 0xF8) { *c = KEY_I; return "I 8"; }
    if (k == 0xA9) { *c = KEY_COMMA; return ", ;"; }
    if (k == 0xC9) { *c = KEY_L; return "L ("; }
    if (k == 0xF9) { *c = KEY_O; return "O 9"; }
    if (k == 0xAA) { *c = KEY_DOT; return ". :"; }
    if (k == 0xCA) { *c = KEY_SLASH; *shift = true; return "? )"; }
    if (k == 0xFA) { *c = KEY_P; return "P 0"; }
    if (k == 0xCB) { *c = KEY_ENTER; return "Return"; }
    if (k == 0xFB) { *c = KEY_BACKSPACE; return "Del"; }
    if (k == 0xFC) { *c = KEY_UP; return "Up arrow"; }
    if (k == 0x9C) { *c = KEY_ENTER; return "OK"; }
    if (k == 0xBD) { *c = KEY_DOWN; return "Down arrow"; }
    if (k == 0xFD) { *c = KEY_LEFT; return "Left Arrow"; }
    if (k == 0x9D) { *c = KEY_RIGHT; return "Right Arrow"; }
    if (k == 0xB9) { *c = KEY_BACKSLASH; return "| Escape"; }

    if (k == 0x00) return "! Released";

    return "* Unknown";
}
