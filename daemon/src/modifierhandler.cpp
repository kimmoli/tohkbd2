#include <stdio.h>
#include "modifierhandler.h"

modifierHandler::modifierHandler(QString name, QObject *parent) :
    QObject(parent)
{
    _name = name;

    mode = Normal;
    pressed = false;
    down = false;
    locked = false;

    _wasHeldDown = false;
    _lockCount = 0;

    printf("modifierHandler for \"%s\"\n", qPrintable(_name));
}

void modifierHandler::set(bool state, bool alone)
{
    bool newPressed = pressed;

    /* In normal mode, just follow the state */
    if (mode == Normal)
    {
        newPressed = state;
    }
    else
    {
         /* sticky or lock, but held while another key is being presed */
        if ((mode == Sticky || mode == Lock) && state && down && !alone)
        {
            _wasHeldDown = true;
        }

        /* If something was pressed while held, release when released */
        if ((mode == Sticky || mode == Lock) && !state && _wasHeldDown)
        {
            _wasHeldDown = false;
            _lockCount = 0;
            locked = false;
            newPressed = false;
        }
        /* Just pressing toggles if sticky */
        else if (mode == Sticky && state && alone && !_wasHeldDown)
        {
            newPressed = !pressed;
        }
            /* Reset lock count if anything else pressed */
        else if (mode == Lock && !locked && !alone && _lockCount != 3)
        {
            _lockCount = 0;
        }
        /* If just modifier key is pressed */
        else if (mode == Lock && alone)
        {
            if (!down && state && (_lockCount == 0))
            {
                newPressed = true;
                _lockCount = 1;
            }
            else if (down && !state && (_lockCount == 1))
            {
                newPressed = false;
                _lockCount = 2;
            }
            else if (!down && state && (_lockCount == 2))
            {
                _lockCount = 3;
                newPressed = true;
                locked = true;
            }
            else if (!down && state && (_lockCount == 3))
            {
                _lockCount = 0;
                newPressed = true;
                locked = false;
            }
            else if (down && !state && (_lockCount == 0))
            {
                newPressed = false;
            }
        }
    }

    down = state;

    /* So, was the pressed -state actually to be changed? */
    if (newPressed != pressed)
    {
        pressed = newPressed;
        printf("%s changed to %s\n", qPrintable(_name), pressed ? "down" : "up");
        emit changed();
    }
}

void modifierHandler::clear(bool force)
{
    /* If there is something to clear, clear */
    if ((pressed && !down && !locked) || force)
    {
        locked = false;
        _lockCount = 0;
        _wasHeldDown = false;

        pressed = false;
        emit changed();
    }
}

void modifierHandler::setMode(KeyMode newMode)
{
    /* Change mode and clear states */
    if (newMode != mode)
    {
        mode = newMode;
        clear(true);
    }
}
