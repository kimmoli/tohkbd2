#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <QObject>
#include <QtDBus/QtDBus>
#include <QDateTime>
#include <QDBusMessage>
#include <nemonotifications-qt5/notification.h>

class ScreenShot : public QObject
{
    Q_OBJECT
public:
    explicit ScreenShot(QObject *parent = 0);

signals:

public slots:
    void takeScreenShot();

private:
    Notification notif;
};

#endif // SCREENSHOT_H
