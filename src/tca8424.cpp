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

/* REV 2 Keyboard mapping
 *
 *    Rows      0     1     2     3     4     5     6     7     8     9     10    11    12    13    14
 *          HEX 1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
 *Colums 0  A   esc         1     2     3     4     5     6     7     8     9     0     -     =     backspace
 *       1  B   del   Up    ins   q     w     e     r     t     y     u     i     o     p     +     ctrl
 *       2  C   <-          ->    a     s     d     f     g     h     j     k     l     ö     ä     alt
 *       3  D   home  down  end   z     x     c     v     b     n     m     ?     !     ,     .
 *       4  E   SYM         ctrl  shift       space space       space shift       @     SYM         return
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
    if (k == 0xE4 || k == 0xEA)
    {
        *shift = true;
    }
    else if (k == 0xCF)
    {
        *alt = true;
    }
    else if (k == 0xE2 || k == 0xBF)
    {
        *ctrl = true;
        return "! Ctrl";
    }

    /* TODO SYM = 0xE1 || 0xED */

    /* if alt, use alternate key mapping */
    if (*alt)
    {
        k = input[6];

        if (k == 0x00) return "! Released";

        return "* Unknown";
    }

    /* if shift, check the next key in map */
    if (*shift)
        k = input[6];

    if (k == 0xA1) { *c = KEY_ESC; return "(A1) esc"; }
    if (k == 0xA3) { *c = KEY_1; return "(A3) 1"; }
    if (k == 0xA4) { *c = KEY_2; return "(A4) 2"; }
    if (k == 0xA5) { *c = KEY_3; return "(A5) 3"; }
    if (k == 0xA6) { *c = KEY_4; return "(A6) 4"; }
    if (k == 0xA7) { *c = KEY_5; return "(A7) 5"; }
    if (k == 0xA8) { *c = KEY_6; return "(A8) 6"; }
    if (k == 0xA9) { *c = KEY_7; return "(A9) 7"; }
    if (k == 0xAA) { *c = KEY_8; return "(AA) 8"; }
    if (k == 0xAB) { *c = KEY_9; return "(AB) 9"; }
    if (k == 0xAC) { *c = KEY_0; return "(AC) 0"; }
    if (k == 0xAD) { *c = KEY_MINUS; return "(AD) -"; }
    if (k == 0xAE) { *c = KEY_EQUAL; return "(AE) ="; }
    if (k == 0xAF) { *c = KEY_BACKSPACE; return "(AF) backspace"; }

    if (k == 0xB1) { *c = KEY_DELETE; return "(B1) del"; }
    if (k == 0xB2) { *c = KEY_UP; return "(B2) Up"; }
    if (k == 0xB3) { *c = KEY_INSERT; return "(B3) insert"; }
    if (k == 0xB4) { *c = KEY_Q; return "(B4) q"; }
    if (k == 0xB5) { *c = KEY_W; return "(B5) w"; }
    if (k == 0xB6) { *c = KEY_E; return "(B6) e"; }
    if (k == 0xB7) { *c = KEY_R; return "(B7) r"; }
    if (k == 0xB8) { *c = KEY_T; return "(B8) t"; }
    if (k == 0xB9) { *c = KEY_Y; return "(B9) y"; }
    if (k == 0xBA) { *c = KEY_U; return "(BA) u"; }
    if (k == 0xBB) { *c = KEY_I; return "(BB) i"; }
    if (k == 0xBC) { *c = KEY_O; return "(BC) o"; }
    if (k == 0xBD) { *c = KEY_P; return "(BD) p"; }
    if (k == 0xBE) { *c = KEY_KPPLUS; return "(BE) +"; }

    if (k == 0xC1) { *c = KEY_LEFT; return "(C1) <-"; }
    if (k == 0xC3) { *c = KEY_RIGHT; return "(C3) ->"; }
    if (k == 0xC4) { *c = KEY_A; return "(C4) a"; }
    if (k == 0xC5) { *c = KEY_S; return "(C5) s"; }
    if (k == 0xC6) { *c = KEY_D; return "(C6) d"; }
    if (k == 0xC7) { *c = KEY_F; return "(C7) f"; }
    if (k == 0xC8) { *c = KEY_G; return "(C8) g"; }
    if (k == 0xC9) { *c = KEY_H; return "(C9) h"; }
    if (k == 0xCA) { *c = KEY_J; return "(CA) j"; }
    if (k == 0xCB) { *c = KEY_K; return "(CB) k"; }
    if (k == 0xCC) { *c = KEY_L; return "(CC) l"; }
//    if (k == 0xCD) { *c = KEY_; return "(CD) ö"; }
//    if (k == 0xCE) { *c = KEY_; return "(CE) ä"; }

    if (k == 0xD1) { *c = KEY_HOME; return "(D1) home"; }
    if (k == 0xD2) { *c = KEY_DOWN; return "(D2) down"; }
    if (k == 0xD3) { *c = KEY_END; return "(D3) end"; }
    if (k == 0xD4) { *c = KEY_Z; return "(D4) z"; }
    if (k == 0xD5) { *c = KEY_X; return "(D5) x"; }
    if (k == 0xD6) { *c = KEY_C; return "(D6) c"; }
    if (k == 0xD7) { *c = KEY_V; return "(D7) v"; }
    if (k == 0xD8) { *c = KEY_B; return "(D8) b"; }
    if (k == 0xD9) { *c = KEY_N; return "(D9) n"; }
    if (k == 0xDA) { *c = KEY_M; return "(DA) m"; }
    if (k == 0xDB) { *c = KEY_0; *shift = true; return "(DB) ? "; }
    if (k == 0xDC) { *c = KEY_1; *shift = true; return "(DC) !"; }
    if (k == 0xDD) { *c = KEY_COMMA; return "(DD) ,"; }
    if (k == 0xDE) { *c = KEY_DOT; return "(DE) ."; }

    if (k == 0xE6) { *c = KEY_SPACE; return "(E6) space"; }
    if (k == 0xE7) { *c = KEY_SPACE; return "(E7) space"; }
    if (k == 0xE9) { *c = KEY_SPACE; return "(E9) space"; }
    if (k == 0xEC) { *c = KEY_2; *shift = true; return "(EC) @"; }
    if (k == 0xEF) { *c = KEY_ENTER; return "(EF) return"; }


    if (k == 0x00) return "! Released";

    return "* Unknown";
}
