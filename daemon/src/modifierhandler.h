#ifndef MODIFIERHANDLER_H
#define MODIFIERHANDLER_H

#include <QObject>

class modifierHandler : public QObject
{
    Q_OBJECT
public:
    explicit modifierHandler(QString name, QObject *parent = 0);

    typedef enum KeyMode
    {
        Normal = 0,
        Sticky,
        Lock
    } KeyMode;

    void set(bool state, bool alone);
    void clear(bool force = false);

    void setMode(KeyMode newMode);

    KeyMode mode;

    bool pressed;
    bool down;
    bool locked;

signals:
    void changed();

public slots:

private:
    QString _name;
    bool _wasHeldDown;
    int _lockCount;

};

#endif // MODIFIERHANDLER_H
