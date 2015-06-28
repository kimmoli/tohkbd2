#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <QObject>
#include <QtDBus/QtDBus>
#include <QDateTime>
#include <QDBusMessage>

class ScreenShot : public QObject
{
    Q_OBJECT
public:
    explicit ScreenShot(QObject *parent = 0);

signals:

public slots:
    void takeScreenShot();
    void handleNotificationActionInvoked(const QDBusMessage& msg);

private:
    void notificationSend(QString summary, QString body);

    unsigned int ssNotifyReplacesId;
    QString ssFilename;
};

#endif // SCREENSHOT_H
