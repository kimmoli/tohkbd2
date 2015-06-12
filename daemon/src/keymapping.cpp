#include "keymapping.h"
#include <linux/input.h>
#include <stdio.h>

keymapping::keymapping(QObject *parent) :
    QObject(parent)
{
    shiftPressed = false;
    ctrlPressed = false;
    altPressed = false;
    symPressed = false;

    stickyCtrlEnabled = false;
    stickyAltEnabled = false;
    stickySymEnabled = false;
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
 *              (02)        (08)  (10)        (40)  (80)
 *  (00) values are modifier values, in 4th byte of inputreport for SYM, ctrl, shift and space
 */

void keymapping::process(QByteArray inputReport)
{
    char key;
    int n;
    QList< QPair<int,int> > retKey;

    bool __shiftPressed = false;
    bool __ctrlPressed = false;
    bool __altPressed = false;
    bool __symPressed = false;

    printf("Processing report: ");
    for (n=0 ; n<inputReport.count() ; n++)
        printf("%02x ", inputReport.at(n));
    printf("\n");

    for (n=3 ; n<11 ; n++)
    {
        if (n == 4) { continue; }

        key = inputReport.at(n);

        if (key == 0x00) { continue; }
        else if (key == 0x10 || key == 0xEA) { __shiftPressed = true; }
        else if (key == 0xCF)                { __altPressed = true; }
        else if (key == 0x08 || key == 0xBF) { __ctrlPressed = true; }
        else if (key == 0x02 || key == 0xED) { __symPressed = true; }
        else if (key == 0xA1) { retKey.append(qMakePair(KEY_ESC, 0)); }
        else if (key == 0xA3) { retKey.append(qMakePair(KEY_1, 0)); }
        else if (key == 0xA4) { retKey.append(qMakePair(KEY_2, 0)); }
        else if (key == 0xA5) { retKey.append(qMakePair(KEY_3, 0)); }
        else if (key == 0xA6) { retKey.append(qMakePair(KEY_4, 0)); }
        else if (key == 0xA7) { retKey.append(qMakePair(KEY_5, 0)); }
        else if (key == 0xA8) { retKey.append(qMakePair(KEY_6, 0)); }
        else if (key == 0xA9) { retKey.append(qMakePair(KEY_7, 0)); }
        else if (key == 0xAA) { retKey.append(qMakePair(KEY_8, 0)); }
        else if (key == 0xAB) { retKey.append(qMakePair(KEY_9, 0)); }
        else if (key == 0xAC) { retKey.append(qMakePair(KEY_0, 0)); }
        else if (key == 0xAD) { retKey.append(qMakePair(KEY_MINUS, 0)); }
        else if (key == 0xAE) { retKey.append(qMakePair(KEY_EQUAL, 0)); }
        else if (key == 0xAF) { retKey.append(qMakePair(KEY_BACKSPACE, 0)); }

        else if (key == 0xB1) { retKey.append(qMakePair(KEY_DELETE, 0)); }
        else if (key == 0xB2) { retKey.append(qMakePair(KEY_UP, 0)); }
        else if (key == 0xB3) { retKey.append(qMakePair(KEY_INSERT, 0)); }
        else if (key == 0xB4) { retKey.append(qMakePair(KEY_Q, 0)); }
        else if (key == 0xB5) { retKey.append(qMakePair(KEY_W, 0)); }
        else if (key == 0xB6) { retKey.append(qMakePair(KEY_E, 0)); }
        else if (key == 0xB7) { retKey.append(qMakePair(KEY_R, 0)); }
        else if (key == 0xB8) { retKey.append(qMakePair(KEY_T, 0)); }
        else if (key == 0xB9) { retKey.append(qMakePair(KEY_Y, 0)); }
        else if (key == 0xBA) { retKey.append(qMakePair(KEY_U, 0)); }
        else if (key == 0xBB) { retKey.append(qMakePair(KEY_I, 0)); }
        else if (key == 0xBC) { retKey.append(qMakePair(KEY_O, 0)); }
        else if (key == 0xBD) { retKey.append(qMakePair(KEY_P, 0)); }
        else if (key == 0xBE) { retKey.append(qMakePair(KEY_KPPLUS, 0)); }

        else if (key == 0xC1) { retKey.append(qMakePair(KEY_LEFT, 0)); }
        else if (key == 0xC3) { retKey.append(qMakePair(KEY_RIGHT, 0)); }
        else if (key == 0xC4) { retKey.append(qMakePair(KEY_A, 0)); }
        else if (key == 0xC5) { retKey.append(qMakePair(KEY_S, 0)); }
        else if (key == 0xC6) { retKey.append(qMakePair(KEY_D, 0)); }
        else if (key == 0xC7) { retKey.append(qMakePair(KEY_F, 0)); }
        else if (key == 0xC8) { retKey.append(qMakePair(KEY_G, 0)); }
        else if (key == 0xC9) { retKey.append(qMakePair(KEY_H, 0)); }
        else if (key == 0xCA) { retKey.append(qMakePair(KEY_J, 0)); }
        else if (key == 0xCB) { retKey.append(qMakePair(KEY_K, 0)); }
        else if (key == 0xCC) { retKey.append(qMakePair(KEY_L, 0)); }
        else if (key == 0xCD) { retKey.append(qMakePair(KEY_SEMICOLON, 0)); /* ; */ }
        else if (key == 0xCE) { retKey.append(qMakePair(KEY_APOSTROPHE, 0)); /* ' */ }

        else if (key == 0xD1) { retKey.append(qMakePair(KEY_HOME, 0)); }
        else if (key == 0xD2) { retKey.append(qMakePair(KEY_DOWN, 0)); }
        else if (key == 0xD3) { retKey.append(qMakePair(KEY_END, 0)); }
        else if (key == 0xD4) { retKey.append(qMakePair(KEY_Z, 0)); }
        else if (key == 0xD5) { retKey.append(qMakePair(KEY_X, 0)); }
        else if (key == 0xD6) { retKey.append(qMakePair(KEY_C, 0)); }
        else if (key == 0xD7) { retKey.append(qMakePair(KEY_V, 0)); }
        else if (key == 0xD8) { retKey.append(qMakePair(KEY_B, 0)); }
        else if (key == 0xD9) { retKey.append(qMakePair(KEY_N, 0)); }
        else if (key == 0xDA) { retKey.append(qMakePair(KEY_M, 0)); }
        else if (key == 0xDB) { retKey.append(qMakePair(KEY_SLASH, FORCE_SHIFT)); /* ? */ }
        else if (key == 0xDC) { retKey.append(qMakePair(KEY_1, FORCE_SHIFT)); /* ! */ }
        else if (key == 0xDD) { retKey.append(qMakePair(KEY_COMMA, 0)); }
        else if (key == 0xDE) { retKey.append(qMakePair(KEY_DOT, 0)); }

        else if (key == 0x40) { retKey.append(qMakePair(KEY_SPACE, 0)); }
        else if (key == 0x80) { retKey.append(qMakePair(KEY_SPACE, 0)); }
        else if (key == 0xE9) { retKey.append(qMakePair(KEY_SPACE, 0)); }
        else if (key == 0xEC) { retKey.append(qMakePair(KEY_2, FORCE_SHIFT)); /* @ */ }
        else if (key == 0xEF) { retKey.append(qMakePair(KEY_ENTER, 0)); }

        if (!retKey.empty())
            break;
    }

    if (__shiftPressed != shiftPressed)
    {
        shiftPressed = __shiftPressed;
        emit shiftChanged();
    }

    /* Toggle ctrl when only ctrl is pressed */
    if (stickyCtrlEnabled && __ctrlPressed && retKey.empty())
    {
        ctrlPressed = !ctrlPressed;
        emit ctrlChanged();
    }
    else if (!stickyCtrlEnabled && (__ctrlPressed != ctrlPressed))
    {
        ctrlPressed = __ctrlPressed;
        emit ctrlChanged();
    }

    if (stickyAltEnabled && __altPressed)
    {
        altPressed = !altPressed;
        emit altChanged();
    }
    else if (__altPressed != altPressed)
    {
        altPressed = __altPressed;
        emit altChanged();
    }

    if (stickySymEnabled && __symPressed)
    {
        symPressed = !symPressed;
        emit symChanged();
    }
    else if (__symPressed != symPressed)
    {
        symPressed = __symPressed;
        emit symChanged();
    }

    if (!retKey.empty())
        emit keyPressed(retKey);

    if (retKey.empty() || (retKey != _prevRetKey && !_prevRetKey.empty()))
        emit keyReleased();

    _prevRetKey = retKey;
}

void keymapping::releaseStickyModifiers()
{
    if (ctrlPressed)
    {
        ctrlPressed = false;
        emit ctrlChanged();
    }
    if (altPressed)
    {
        altPressed = false;
        emit altChanged();
    }
    if (symPressed)
    {
        symPressed = false;
        emit symChanged();
    }
}
