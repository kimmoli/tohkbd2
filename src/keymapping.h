#ifndef KEYMAPPING_H
#define KEYMAPPING_H

#include <QObject>
#include <QList>
#include <QPair>

#define FORCE_SHIFT    (1)
#define FORCE_ALT      (2)
#define FORCE_RIGHTALT (4)
#define FORCE_COMPOSE  (8)
#define KEEP           (0x10)

class keymapping : public QObject
{
    Q_OBJECT
public:
    explicit keymapping(QObject *parent = 0);

    void process(QByteArray inputReport);

    bool shiftPressed;
    bool ctrlPressed;
    bool altPressed;
    bool symPressed;

signals:
    void shiftChanged();
    void ctrlChanged();
    void altChanged();
    void symChanged();
    void keyPressed(QList< QPair<int, int> > keyCode);
    void keyReleased();

public slots:

private:

};

#endif // KEYMAPPING_H
