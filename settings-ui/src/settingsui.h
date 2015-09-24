/*
    tohkbd2-settings-u, The Otherhalf Keyboard 2 settings UI
*/

#ifndef SETTINGSUI_H
#define SETTINGSUI_H
#include <QObject>
#include <QVariantList>
#include <QTimer>
#include <QMap>

#include "../dbus/src/daemonInterface.h"
#include "../dbus/src/userdaemonInterface.h"

#define SERVICE_NAME "com.kimmoli.tohkbd2settingsui"

class QDBusInterface;
class SettingsUi : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", SERVICE_NAME)
    Q_PROPERTY(QString version READ readVersion NOTIFY versionChanged())

public:
    explicit SettingsUi(QObject *parent = 0);
    ~SettingsUi();
    void registerDBus();

    QString readVersion();
    Q_INVOKABLE QVariantList getApplications();
    Q_INVOKABLE QVariantList getCurrentShortcuts();
    Q_INVOKABLE QVariantMap getCurrentSettings();
    Q_INVOKABLE QVariantList getCurrentLayouts();
    Q_INVOKABLE void setShortcut(QString key, QString appPath);
    Q_INVOKABLE QString readDaemonVersion();
    Q_INVOKABLE QString readUserDaemonVersion();
    Q_INVOKABLE QString readSailfishVersion();
    Q_INVOKABLE void setShortcutsToDefault();
    Q_INVOKABLE void setSettingsToDefault();
    Q_INVOKABLE void setSetting(QString key, QVariant value);
    Q_INVOKABLE void forceKeymapReload();
    Q_INVOKABLE void restoreOriginalKeymaps();

public slots:
    void showHelp() { emit showHelpPage(); }

signals:
    void versionChanged();
    void shortcutsChanged();
    void settingsChanged();
    void showHelpPage();

private slots:
    void handlePhysicalLayoutChange(QString layout);

private:

    ComKimmoliTohkbd2Interface *tohkbd2daemon;
    ComKimmoliTohkbd2userInterface *tohkbd2user;

    bool m_dbusRegistered;
    QMap<QString, QString> layoutToLanguage;
};


#endif // SETTINGSUI_H

