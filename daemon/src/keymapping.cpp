#include "keymapping.h"
#include <linux/input.h>
#include <stdio.h>
#include "keymapping_lut.h"

keymapping::keymapping(QObject *parent) :
    QObject(parent)
{
    shiftPressed = false;
    ctrlPressed = false;
    altPressed = false;
    symPressed = false;
    keyIsPressed = false;

    stickyCtrlEnabled = false;
    stickyAltEnabled = false;
    stickySymEnabled = false;

    ctrlDown = false;
    altDown = false;
    symDown = false;

    _prevScanCode = 0;
    _prevInputReport = QByteArray();
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
    int n;
    QList< QPair<int,int> > retKey;
    char irCode = 0;

    bool __shiftPressed = false;
    ctrlDown = false;
    altDown = false;
    symDown = false;

    printf("Processing report: ");
    for (n=0 ; n<inputReport.count() ; n++)
        printf("%02x ", inputReport.at(n));
    printf("\n");


    QByteArray ir = inputReport.mid(5, 6);

    /* Remove empty usage code bytes */
    int j;
    while ((j = ir.indexOf((char)0x00)) != -1)
        ir.remove(j, 1);

    /* First check modifiers from modifier byte */
    if (inputReport.at(3) & 0x02) symDown = true;
    if (inputReport.at(3) & 0x08) ctrlDown = true;
    if (inputReport.at(3) & 0x10) __shiftPressed = true;
    /* And other modifiers from the usage codes */
    if (ir.contains(0xEA)) { __shiftPressed = true; ir.remove(ir.indexOf(0xEA), 1); }
    if (ir.contains(0xCF)) { altDown = true; ir.remove(ir.indexOf(0xCF), 1); }
    if (ir.contains(0xBF)) { ctrlDown = true; ir.remove(ir.indexOf(0xBF), 1); }
    if (ir.contains(0xED)) { symDown = true; ir.remove(ir.indexOf(0xED), 1); }

    /* Check space key here, it is a special case */
    if ((inputReport.at(3) & 0x40) || (inputReport.at(3) & 0x80) || ir.contains(0xE9))
    {
        if (!ir.contains(0xE9))
            ir.append(0xE9);
    }

    if (__shiftPressed != shiftPressed)
    {
        shiftPressed = __shiftPressed;
        emit shiftChanged();
    }

    /* CTRL */
    if (stickyCtrlEnabled && ctrlDown && ctrlPressed && !ir.isEmpty())
    {
        ctrlWasHeldDown = true;
    }

    if (stickyCtrlEnabled && ctrlDown && ir.isEmpty() && !keyIsPressed)
    {
        releaseStickyModifiers();
        ctrlPressed = !ctrlPressed;
        emit ctrlChanged();
    }
    else if (!stickyCtrlEnabled && (ctrlDown != ctrlPressed))
    {
        ctrlPressed = ctrlDown;
        emit ctrlChanged();
    }

    /* ALT */
    if (stickyAltEnabled && altDown && altPressed && !ir.isEmpty())
    {
        altWasHeldDown = true;
    }

    if (stickyAltEnabled && altDown && ir.isEmpty() && !keyIsPressed)
    {
        releaseStickyModifiers();
        altPressed = !altPressed;
        emit altChanged();
    }
    else if (!stickyAltEnabled && (altDown != altPressed))
    {
        altPressed = altDown;
        emit altChanged();
    }

    /* SYM */
    if (stickySymEnabled && symDown && symPressed && !ir.isEmpty())
    {
        symWasHeldDown = true;
    }

    if (stickySymEnabled && symDown && ir.isEmpty() && !keyIsPressed)
    {
        releaseStickyModifiers();
        symPressed = !symPressed;
        emit symChanged();
    }
    else if (!stickySymEnabled && (symDown != symPressed))
    {
        symPressed = symDown;
        emit symChanged();
    }

    printf("There are %d usage codes to process\n", ir.length());

    /* Shortcut out if no actual key pressed */
    if (ir.length() == 0)
    {
        if (ctrlWasHeldDown && !ctrlDown && ctrlPressed)
        {
            ctrlWasHeldDown = false;
            ctrlPressed = false;
            emit ctrlChanged();
        }
        if (altWasHeldDown && !altDown && altPressed)
        {
            altWasHeldDown = false;
            altPressed = false;
            emit altChanged();
        }
        if (symWasHeldDown && !symDown && symPressed)
        {
            symWasHeldDown = false;
            symPressed = false;
            emit symChanged();
        }

        if (keyIsPressed)
        {
            keyIsPressed = false;
            _prevScanCode = 0;
            emit keyReleased();
        }
        _prevInputReport = ir;
        return;
    }

    /* Check for new code in report */

    for(int i = ir.length()-1 ; i>=0 ; --i)
    {
        if (!_prevInputReport.contains(ir.at(i)))
        {
            irCode = ir.at(i);
            break;
        }
    }

    if (!symPressed && irCode>0) /* Without SYM modifier */
    {
        int i = 0;
        while (lut_plain[i])
        {
            if (irCode == lut_plain[i])
            {
                 retKey.append(qMakePair(lut_plain[i+1], lut_plain[i+2]));
                 break;
            }
            i += 3;
        }
    }
    else if (symPressed && irCode>0) /* With SYM modifier */
    {
        int i = 0;
        while (lut_sym[i])
        {
            if (irCode == lut_sym[i])
            {
                 retKey.append(qMakePair(lut_sym[i+1], lut_sym[i+2]));
                 break;
            }
            i += 3;
        }
    }

    /* If key is changed on the fly without break... emit released */
    if (keyIsPressed && _prevScanCode > 0 && !retKey.empty())
    {
        if (_prevScanCode != irCode)
            emit keyReleased();
        if (_prevScanCode == irCode)
            return;
    }

    /* Key, not a modifier, is pressed */
    keyIsPressed = !retKey.empty();

    if (keyIsPressed)
        emit keyPressed(retKey);

    _prevScanCode = irCode;
    _prevInputReport = ir;
}

void keymapping::releaseStickyModifiers()
{
    if (ctrlPressed && !ctrlDown)
    {
        ctrlPressed = false;
        emit ctrlChanged();
    }
    if (altPressed && !altDown)
    {
        altPressed = false;
        emit altChanged();
    }
    if (symPressed && !symDown)
    {
        symPressed = false;
        emit symChanged();
    }
}
