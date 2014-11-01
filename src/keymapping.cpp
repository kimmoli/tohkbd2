#include "keymapping.h"
#include <linux/input.h>

keymapping::keymapping(QObject *parent) :
    QObject(parent)
{
    shiftPressed = false;
    ctrlPressed = false;
    altPressed = false;
    symPressed = false;
}

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

void keymapping::process(QByteArray inputReport)
{
    char key;
    int retKey = KEY_RESERVED;
    bool forceShift = false;

    bool __shiftPressed = false;
    bool __ctrlPressed = false;
    bool __altPressed = false;
    bool __symPressed = false;

    for (int n=5 ; n<11 ; n++)
    {
        key = inputReport.at(n);

        if (key == 0x00) { break; }
        else if (key == 0xE4 || key == 0xEA) { __shiftPressed = true; }
        else if (key == 0xCF)                { __altPressed = true; }
        else if (key == 0xE2 || key == 0xBF) { __ctrlPressed = true; }
        else if (key == 0xE1 || key == 0xED) { __symPressed = true; }
        else if (key == 0xA1) { retKey = KEY_ESC; }
        else if (key == 0xA3) { retKey = KEY_1; }
        else if (key == 0xA4) { retKey = KEY_2; }
        else if (key == 0xA5) { retKey = KEY_3; }
        else if (key == 0xA6) { retKey = KEY_4; }
        else if (key == 0xA7) { retKey = KEY_5; }
        else if (key == 0xA8) { retKey = KEY_6; }
        else if (key == 0xA9) { retKey = KEY_7; }
        else if (key == 0xAA) { retKey = KEY_8; }
        else if (key == 0xAB) { retKey = KEY_9; }
        else if (key == 0xAC) { retKey = KEY_0; }
        else if (key == 0xAD) { retKey = KEY_MINUS; }
        else if (key == 0xAE) { retKey = KEY_EQUAL; }
        else if (key == 0xAF) { retKey = KEY_BACKSPACE; }

        else if (key == 0xB1) { retKey = KEY_DELETE; }
        else if (key == 0xB2) { retKey = KEY_UP; }
        else if (key == 0xB3) { retKey = KEY_INSERT; }
        else if (key == 0xB4) { retKey = KEY_Q; }
        else if (key == 0xB5) { retKey = KEY_W; }
        else if (key == 0xB6) { retKey = KEY_E; }
        else if (key == 0xB7) { retKey = KEY_R; }
        else if (key == 0xB8) { retKey = KEY_T; }
        else if (key == 0xB9) { retKey = KEY_Y; }
        else if (key == 0xBA) { retKey = KEY_U; }
        else if (key == 0xBB) { retKey = KEY_I; }
        else if (key == 0xBC) { retKey = KEY_O; }
        else if (key == 0xBD) { retKey = KEY_P; }
        else if (key == 0xBE) { retKey = KEY_KPPLUS; }

        else if (key == 0xC1) { retKey = KEY_LEFT; }
        else if (key == 0xC3) { retKey = KEY_RIGHT; }
        else if (key == 0xC4) { retKey = KEY_A; }
        else if (key == 0xC5) { retKey = KEY_S; }
        else if (key == 0xC6) { retKey = KEY_D; }
        else if (key == 0xC7) { retKey = KEY_F; }
        else if (key == 0xC8) { retKey = KEY_G; }
        else if (key == 0xC9) { retKey = KEY_H; }
        else if (key == 0xCA) { retKey = KEY_J; }
        else if (key == 0xCB) { retKey = KEY_K; }
        else if (key == 0xCC) { retKey = KEY_L; }
    //    else if (key == 0xCD) { retKey = KEY_; /* Ö */ }
    //    else if (key == 0xCE) { retKey = KEY_; /* Ä */ }

        else if (key == 0xD1) { retKey = KEY_HOME; }
        else if (key == 0xD2) { retKey = KEY_DOWN; }
        else if (key == 0xD3) { retKey = KEY_END; }
        else if (key == 0xD4) { retKey = KEY_Z; }
        else if (key == 0xD5) { retKey = KEY_X; }
        else if (key == 0xD6) { retKey = KEY_C; }
        else if (key == 0xD7) { retKey = KEY_V; }
        else if (key == 0xD8) { retKey = KEY_B; }
        else if (key == 0xD9) { retKey = KEY_N; }
        else if (key == 0xDA) { retKey = KEY_M; }
        else if (key == 0xDB) { retKey = KEY_0; forceShift = true; /* ? */ }
        else if (key == 0xDC) { retKey = KEY_1; forceShift = true; /* ! */ }
        else if (key == 0xDD) { retKey = KEY_COMMA; }
        else if (key == 0xDE) { retKey = KEY_DOT; }

        else if (key == 0xE6) { retKey = KEY_SPACE; }
        else if (key == 0xE7) { retKey = KEY_SPACE; }
        else if (key == 0xE9) { retKey = KEY_SPACE; }
        else if (key == 0xEC) { retKey = KEY_2; forceShift = true; /* @ */ }
        else if (key == 0xEF) { retKey = KEY_ENTER; }

        if (retKey != KEY_RESERVED)
            break;
    }

    if (__shiftPressed != shiftPressed)
    {
        shiftPressed = __shiftPressed;
        emit shiftChanged();
    }

    if (__ctrlPressed != ctrlPressed)
    {
        ctrlPressed = __ctrlPressed;
        emit ctrlChanged();
    }

    if (__altPressed != altPressed)
    {
        altPressed = __altPressed;
        emit altChanged();
    }

    if (__symPressed != symPressed)
    {
        symPressed = __symPressed;
        emit symChanged();
    }

    if (retKey != KEY_RESERVED)
        emit keyPressed(retKey, forceShift);
}
