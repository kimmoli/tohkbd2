/*
 * (C) 2014 Kimmo Lindholm <kimmo.lindholm@gmail.com> Kimmoli
 *
 */

#ifndef _TOHKBD_TCA8424_H
#define _TOHKBD_TCA8424_H 1

#define TCA_ADDR 0x3b

int tca8424_readInputReport(int file, unsigned char *report);
int tca8424_initComms(unsigned char addr);
int tca8424_closeComms(int file);
int tca8424_reset(int file);
int tca8424_leds(int file, unsigned char leds);
int tca8424_readMemory(int file, int start, int len, unsigned char *data);
const char* tca8424_processKeyMap(unsigned char *input, int *code, int *isShift, int *isAlt, int *isCtrl);

#endif // !__TOHKBD_TCA8424_H
