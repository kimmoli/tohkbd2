#include <QStringList>
#include <QFile>
#include <QTextStream>

#include "keymapping.h"
#include <linux/input.h>
#include <stdio.h>

QStringList keymapping::keyNames = QStringList()
    << "KEY_RESERVED" << "KEY_ESC" << "KEY_1" << "KEY_2" << "KEY_3" << "KEY_4" << "KEY_5" << "KEY_6" << "KEY_7" << "KEY_8" << "KEY_9"
    << "KEY_0" << "KEY_MINUS" << "KEY_EQUAL" << "KEY_BACKSPACE" << "KEY_TAB" << "KEY_Q" << "KEY_W" << "KEY_E" << "KEY_R" << "KEY_T"
    << "KEY_Y" << "KEY_U" << "KEY_I" << "KEY_O" << "KEY_P" << "KEY_LEFTBRACE" << "KEY_RIGHTBRACE" << "KEY_ENTER" << "KEY_LEFTCTRL"
    << "KEY_A" << "KEY_S" << "KEY_D" << "KEY_F" << "KEY_G" << "KEY_H" << "KEY_J" << "KEY_K" << "KEY_L" << "KEY_SEMICOLON" << "KEY_APOSTROPHE"
    << "KEY_GRAVE" << "KEY_LEFTSHIFT" << "KEY_BACKSLASH" << "KEY_Z" << "KEY_X" << "KEY_C" << "KEY_V" << "KEY_B" << "KEY_N" << "KEY_M"
    << "KEY_COMMA" << "KEY_DOT" << "KEY_SLASH" << "KEY_RIGHTSHIFT" << "KEY_KPASTERISK" << "KEY_LEFTALT" << "KEY_SPACE" << "KEY_CAPSLOCK"
    << "KEY_F1" << "KEY_F2" << "KEY_F3" << "KEY_F4" << "KEY_F5" << "KEY_F6" << "KEY_F7" << "KEY_F8" << "KEY_F9" << "KEY_F10" << "KEY_NUMLOCK"
    << "KEY_SCROLLLOCK" << "KEY_KP7" << "KEY_KP8" << "KEY_KP9" << "KEY_KPMINUS" << "KEY_KP4" << "KEY_KP5" << "KEY_KP6" << "KEY_KPPLUS"
    << "KEY_KP1" << "KEY_KP2" << "KEY_KP3" << "KEY_KP0" << "KEY_KPDOT" << "KEY_RESERVED" << "KEY_ZENKAKUHANKAKU" << "KEY_102ND" << "KEY_F11"
    << "KEY_F12" << "KEY_RO" << "KEY_KATAKANA" << "KEY_HIRAGANA" << "KEY_HENKAN" << "KEY_KATAKANAHIRAGANA" << "KEY_MUHENKAN" << "KEY_KPJPCOMMA"
    << "KEY_KPENTER" << "KEY_RIGHTCTRL" << "KEY_KPSLASH" << "KEY_SYSRQ" << "KEY_RIGHTALT" << "KEY_LINEFEED" << "KEY_HOME" << "KEY_UP"
    << "KEY_PAGEUP" << "KEY_LEFT" << "KEY_RIGHT" << "KEY_END" << "KEY_DOWN" << "KEY_PAGEDOWN" << "KEY_INSERT" << "KEY_DELETE" << "KEY_MACRO"
    << "KEY_MUTE" << "KEY_VOLUMEDOWN" << "KEY_VOLUMEUP" << "KEY_POWER" << "KEY_KPEQUAL" << "KEY_KPPLUSMINUS" << "KEY_PAUSE" << "KEY_SCALE"
    << "KEY_KPCOMMA" << "KEY_HANGEUL" << "KEY_HANJA" << "KEY_YEN" << "KEY_LEFTMETA" << "KEY_RIGHTMETA" << "KEY_COMPOSE"
    << "KEY_TOH_TABLE_DELIMITER" /* Keys after this are custom keys */
    << "KEY_TOH_SCREENSHOT" << "KEY_TOH_SELFIE" << "KEY_TOH_NEWEMAIL" << "KEY_TOH_BACKLIGHT";

keymapping::keymapping(QString pathToLayouts, QObject *parent) :
    QObject(parent)
{
    layoutPath = pathToLayouts;

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

    for (j=0; j<ir.length(); j++) /* Quickly check does input report contain bogus */
        if (ir.at(j) < 0xa0)
        {
            printf("keymap: bogus value on input report detected. Resetting TCA\n");
            emit bogusDetected();
            return;
        }

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

    if (leftShiftDown && sym->pressed)
    {
        releaseStickyModifiers();
        emit toggleCapsLock();
        return;
    }

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

bool keymapping::setLayout(QString toLayout)
{
    bool ret = true;

    if (toLayout == layout)
        return true;

    int i = 0;

    QString filename = layoutPath + "/" + toLayout + ".tohkbdmap";

    printf("keymap: reading file %s\n", qPrintable(filename));

    QFile inputFile( filename );

    if ( inputFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
       QTextStream in( &inputFile );
       while (!in.atEnd())
       {
            QStringList line = in.readLine().split(QRegExp("\\s+"));

            // Comment starts with #
            if (line.at(0).startsWith("#"))
               continue;

            if (line.count() != 5)
               continue;

            /*
             * CODE PlainKey PlainModifier SymKey SymModifier
             */

            bool ok;
            int code = line.at(0).toInt(&ok, 16);

            if (!ok)
            {
               printf("keymap: error parsing %s\n", qPrintable(line.at(0)));
               ret = false;
               break;
            }

            lut_plain[i] = code;
            lut_sym[i] = code;

            i++;

            int indexOf = keyNames.indexOf(line.at(1));
            if (indexOf < 0)
            {
               printf("keymap: error parsing %s\n", qPrintable(line.at(1)));
               ret = false;
               break;
            }

            /* Custom keys */
            if (line.at(1).startsWith("KEY_TOH_"))
                indexOf = indexOf - keyNames.indexOf("KEY_TOH_TABLE_DELIMITER") + KEY_MAX;

            lut_plain[i] = indexOf;

            indexOf = keyNames.indexOf(line.at(3));
            if (indexOf < 0)
            {
               printf("keymap: error parsing %s\n", qPrintable(line.at(3)));
               ret = false;
               break;
            }

            if (line.at(3).startsWith("KEY_TOH_"))
                indexOf = indexOf - keyNames.indexOf("KEY_TOH_TABLE_DELIMITER") + KEY_MAX;

            lut_sym[i] = indexOf;

            i++;

            lut_plain[i] = 0;
            lut_sym[i] = 0;

            if (line.at(2).contains("SHIFT"))
                lut_plain[i] |= FORCE_SHIFT;

            if (line.at(4).contains("SHIFT"))
                lut_sym[i] |= FORCE_SHIFT;

            i++;
       }
    }
    else
    {
        printf("keymap: failed to open file\n");
        ret = false;
    }

    /* Fail if not a single key was defined */
    if (i < 3)
    {
        ret = false;
    }

    inputFile.close();

    for ( ; i<256 ; i++)
    {
        lut_plain[i] = 0;
        lut_sym[i] = 0;
    }

    if (ret)
    {
        layout = toLayout;

        printf("keymap: layout set to %s\n", qPrintable(layout));
    }
    else
    {
        printf("keymap: failed to set layout to %s\n", qPrintable(toLayout));
    }

    return ret;
}
