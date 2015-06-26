#include "keymapping.h"
#include <linux/input.h>
#include <stdio.h>
#include "keymapping_lut.h"

keymapping::keymapping(QObject *parent) :
    QObject(parent)
{
    pressedCode = 0;

    shift = new modifierHandler("shift");
    ctrl = new modifierHandler("ctrl");
    alt = new modifierHandler("alt");
    sym = new modifierHandler("sym");

    connect(shift, SIGNAL(changed()), this, SIGNAL(shiftChanged()));
    connect(ctrl, SIGNAL(changed()), this, SIGNAL(ctrlChanged()));
    connect(alt, SIGNAL(changed()), this, SIGNAL(altChanged()));
    connect(sym, SIGNAL(changed()), this, SIGNAL(symChanged()));
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

    bool leftShiftDown = false;
    bool shiftDown = false;
    bool ctrlDown = false;
    bool altDown = false;
    bool symDown = false;

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
    if (inputReport.at(3) & 0x10) leftShiftDown = true;
    /* And other modifiers from the usage codes */
    if (ir.contains(0xEA)) { shiftDown = true; ir.remove(ir.indexOf(0xEA), 1); }
    if (ir.contains(0xCF)) { altDown = true; ir.remove(ir.indexOf(0xCF), 1); }
    if (ir.contains(0xBF)) { ctrlDown = true; ir.remove(ir.indexOf(0xBF), 1); }
    if (ir.contains(0xED)) { symDown = true; ir.remove(ir.indexOf(0xED), 1); }

    /* Check space key here, it is a special case */
    if ((inputReport.at(3) & 0x40) || (inputReport.at(3) & 0x80) || ir.contains(0xE9))
    {
        if (!ir.contains(0xE9))
            ir.append(0xE9);
    }

    if (leftShiftDown && symDown)
        emit toggleCapsLock();

    shift->set(leftShiftDown || shiftDown, ir.isEmpty());
    ctrl->set(ctrlDown, ir.isEmpty());
    alt->set(altDown, ir.isEmpty());
    sym->set(symDown, ir.isEmpty());

    /* Shortcut out if no actual key pressed */
    if (ir.length() == 0)
    {
        if (pressedCode)
        {
            pressedCode = 0;
            emit keyReleased();
        }
        _prevInputReport = ir;
        return;
    }

    /* Check for new code in report. */
    for (int i=ir.length()-1 ; i >= 0 ; --i)
    {
        if (!_prevInputReport.contains(ir.at(i)))
        {
            irCode = ir.at(i);
                break;
        }
    }

    if (sym->pressed && irCode) /* With SYM modifier */
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
    else if (irCode) /* Without SYM modifier */
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


    /* If key is changed on the fly without break... emit released */
    if (pressedCode)
    {
        if ( (!ir.contains(pressedCode)) || ((irCode) && (pressedCode != irCode)) )
        {
            pressedCode = 0;
            emit keyReleased();
        }
        if (_prevInputReport == ir)
            return;
    }

    if (!retKey.empty())
    {
        pressedCode = irCode;
        emit keyPressed(retKey);
    }

    _prevInputReport = ir;
}

void keymapping::releaseStickyModifiers(bool force)
{
    shift->clear(force);
    ctrl->clear(force);
    alt->clear(force);
    sym->clear(force);
}

void keymapping::setLayout(QString toLayout)
{
    if (toLayout == layout)
        return;

    int i = 0;

    if (toLayout == "Scandic")
    {
        while (lut_plain_scandic[i])
        {
            lut_plain[i] = lut_plain_scandic[i];
            i++;
            lut_plain[i] = lut_plain_scandic[i];
            i++;
            lut_plain[i] = lut_plain_scandic[i];
            i++;
        }
    }
    else if (toLayout == "QWERTZ")
    {
        while (lut_plain_qwertz[i])
        {
            lut_plain[i] = lut_plain_qwertz[i];
            i++;
            lut_plain[i] = lut_plain_qwertz[i];
            i++;
            lut_plain[i] = lut_plain_qwertz[i];
            i++;
        }
    }
    else if (toLayout == "AZERTY")
    {
        while (lut_plain_azerty[i])
        {
            lut_plain[i] = lut_plain_azerty[i];
            i++;
            lut_plain[i] = lut_plain_azerty[i];
            i++;
            lut_plain[i] = lut_plain_azerty[i];
            i++;
        }
    }

    for ( ; i<256 ; i++)
        lut_plain[i] = 0;

    layout = toLayout;

    printf("keymap: layout set to %s\n", qPrintable(layout));
}
