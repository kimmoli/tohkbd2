#ifndef UserDaemon_H
#define UserDaemon_H

#include <QObject>
#include <QtDBus/QtDBus>
#include <mlite5/MGConfItem>
#include <nemonotifications-qt5/notification.h>
#include <mlite5/MDesktopEntry>

#define SERVICE_NAME "com.kimmoli.tohkbd2user"
#define KEYMAP_FOLDER "/harbour-tohkbd2"

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
    void _requestActionWithRemorse(const QString &action);
    void _takeScreenShot();
    void physicalLayoutChanged(QString layout);

public slots:
    QString getActiveLayout();
    void setActiveLayout(const QString &value);
    QString getOrientationLock();
    void setOrientationLock(const QString &value);
    void launchApplication(const QString &desktopFilename);
    void showKeyboardConnectionNotification(const bool &connected);
    QString getVersion();
    void showTaskSwitcher() { emit  _showTaskSwitcher(); }
    void nextAppTaskSwitcher() { emit _nextAppTaskSwitcher(); }
    void hideTaskSwitcher() { emit  _hideTaskSwitcher(); }
    void actionWithRemorse(const QString &action);
    void takeScreenShot() { emit _takeScreenShot(); }
    QString getActivePhysicalLayout();
    QString getPathTo(const QString &filename);
    void showUnsupportedLayoutNotification();
    void installKeymaps(const bool &overwrite);
    void setKeymapLayout(const QString &value);
    void setKeymapVariant(const QString &value);
    void quit();

    void launchSuccess(const QString &appName);
    void launchFailed();
    void handlePhysicalLayout();

private:
    void showNotification(const QString &text);
    bool m_dbusRegistered;
    bool m_launchPending;
    MGConfItem *physicalLayout;
};

#endif // UserDaemon_H
