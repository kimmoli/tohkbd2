#ifndef KEYMAPPING_H
#define KEYMAPPING_H

#include <QObject>
#include <QList>
#include <QPair>
#include <QMap>
#include <linux/input.h>
#include "modifierhandler.h"

#define FORCE_SHIFT    (1)
#define FORCE_ALT      (2)
#define FORCE_RIGHTALT (4)
#define FORCE_COMPOSE  (8)
#define KEEP           (0x10)
#define FORCE_CTRL     (0x20)

// Custom keys
#define KEY_TOH_SCREENSHOT (KEY_MAX+1)
#define KEY_TOH_SELFIE     (KEY_MAX+2)
#define KEY_TOH_NEWEMAIL   (KEY_MAX+3)
#define KEY_TOH_BACKLIGHT  (KEY_MAX+4)

class keymapping : public QObject
{
    Q_OBJECT
public:
    explicit keymapping(QString pathToLayouts, QObject *parent = 0);

    void process(QByteArray inputReport);

    void releaseStickyModifiers(bool force = false);

    modifierHandler *shift;
    modifierHandler *ctrl;
    modifierHandler *alt;
    modifierHandler *sym;

    bool setLayout(QString toLayout, bool forceReload = false);

signals:
    void shiftChanged();
    void ctrlChanged();
    void altChanged();
    void symChanged();
    void keyPressed(QList< QPair<int, int> > keyCode);
    void keyReleased();
    void toggleCapsLock();
    void bogusDetected();

public slots:

private:
    bool keyIsPressed;

    char pressedCode;
    QByteArray _prevInputReport;

    QString layout;
    QString layoutPath;

    QMap< int, QPair<int, int> > lut_plain;
    QMap< int, QPair<int, int> > lut_sym;

    static QStringList keyNames;
};

#endif // KEYMAPPING_H
