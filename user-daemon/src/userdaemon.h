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
    void _showTaskSwitcher();
    void _nextAppTaskSwitcher();
    void _hideTaskSwitcher();
    void _lauchApplication(const QString &desktoFilename);
    void _requestReboot();
    void _takeScreenShot();

public slots:
    QString getOrientationLock();
    void setOrientationLock(const QString &value);
    void launchApplication(const QString &desktopFilename);
    void showKeyboardConnectionNotification(const bool &connected);
    QString getVersion();
    void showTaskSwitcher() { emit  _showTaskSwitcher(); }
    void nextAppTaskSwitcher() { emit _nextAppTaskSwitcher(); }
    void hideTaskSwitcher() { emit  _hideTaskSwitcher(); }
    void resetWithRemorse();
    void takeScreenShot() { emit _takeScreenShot(); }
    void quit();

    void launchSuccess(const QString &appName);
    void launchFailed();

private:
    void showNotification(const QString &text);
    bool m_dbusRegistered;
    bool m_launchPending;
};

#endif // UserDaemon_H
