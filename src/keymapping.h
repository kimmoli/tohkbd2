#ifndef KEYMAPPING_H
#define KEYMAPPING_H

#include <QObject>

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
    void keyPressed(int keyCode, bool forceShift);

public slots:

private:

};

#endif // KEYMAPPING_H
