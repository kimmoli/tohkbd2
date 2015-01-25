/*
 * (C) 2014 Kimmo Lindholm <kimmo.lindholm@gmail.com> Kimmoli
 *
 */

#ifndef _TOHKBD_TOH_H
#define _TOHKBD_TOH_H 1

#define GPIO_INT "67"
#define GPIO_INT_EDGE "falling"

int controlVdd(int state);
int getTohInterrupt();
int releaseTohInterrupt(int fdGpio);
unsigned int getEepromConfig(int number);

#endif // !_TOHKBD_TOH_H
