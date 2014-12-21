#ifndef UserDaemon_H
#define UserDaemon_H

#include <QObject>
#include <QtDBus/QtDBus>
#include <mlite5/MGConfItem>
#include <mlite5/MNotification>
#include <mlite5/MDesktopEntry>

#define SERVICE_NAME "com.kimmoli.tohkbd2user"

class QDBusInterface;
class UserDaemon : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", SERVICE_NAME)

public:
    explicit UserDaemon(QObject *parent = 0);
    virtual ~UserDaemon();
    void registerDBus();

signals:

public slots:
    QString getActiveLayout();
    void setActiveLayout(const QString &value);
    void launchApplication(const QString &desktopFilename);
    void showKeyboardConnectionNotification(const bool &connected);
    void quit();

private:
    bool m_dbusRegistered;
    void showNotification(QString text);

};

#endif // UserDaemon_H
