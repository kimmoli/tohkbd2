#ifndef MODIFIERHANDLER_H
#define MODIFIERHANDLER_H

#include <QObject>
#include <QStringList>

class modifierHandler : public QObject
{
    Q_OBJECT
public:
    explicit modifierHandler(QString name, QObject *parent = 0);

    typedef enum KeyMode
    {
        Normal = 0,
        Sticky,
        Lock,
        Cycle
    } KeyMode;

    static QStringList KeyModeNames;
    static KeyMode toKeyMode(const QString &modename);

    void setMode(KeyMode newMode);
    KeyMode mode;

    void set(bool state, bool alone);
    void clear(bool force = false);

    bool pressed;
    bool down;
    bool locked;

    bool verboseMode;

signals:
    void changed();

public slots:

private:
    QString _name;
    bool _wasHeldDown;
    int _lockCount;
};

#endif // MODIFIERHANDLER_H
