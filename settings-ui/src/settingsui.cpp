/*
    tohkbd2-settings-u, The Otherhalf Keyboard 2 settings UI
*/

#include "settingsui.h"
#include <QDir>
#include <QVariantMap>
#include <QVariantList>
#include <QProcess>
#include <QThread>
#include <QSettings>
#include <QDebug>
#include <QtDBus/QtDBus>
#include <algorithm>

#include <mlite5/MDesktopEntry>

#include <linux/input.h>

SettingsUi::SettingsUi(QObject *parent) :
    QObject(parent)
{
    emit versionChanged();
}

SettingsUi::~SettingsUi()
{
}

QString SettingsUi::readVersion()
{
    return APPVERSION;
}

bool appNameLessThan(const QVariant &v1, const QVariant &v2)
{
    return v1.toMap()["name"].toString() < v2.toMap()["name"].toString();
}

QVariantList SettingsUi::getApplications()
{
    QVariantList tmp;
    QVariantMap map;
    QFileInfoList list;
    QDir dir;

    dir.setPath("/usr/share/applications/");
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    dir.setNameFilters(QStringList() << "*.desktop");
    dir.setSorting(QDir::Name);

    list = dir.entryInfoList();

    for (int i=0 ; i<list.size() ; i++)
    {
        MDesktopEntry app(list.at(i).absoluteFilePath());

        if (!app.hidden() && !app.icon().isEmpty() && !app.noDisplay() && !app.notShowIn().contains("X-Meego"))
        {
            map.clear();
            map.insert("filePath", list.at(i).absoluteFilePath());
            map.insert("name", app.name());
            if (app.icon().startsWith("icon-launcher-") || app.icon().startsWith("icon-l-") || app.icon().startsWith("icons-Applications"))
                map.insert("iconId", QString("image://theme/%1").arg(app.icon()));
            else if (app.icon().startsWith("/"))
                map.insert("iconId", QString("%1").arg(app.icon()));
            else
                map.insert("iconId", QString("/usr/share/icons/hicolor/86x86/apps/%1.png").arg(app.icon()));

            map.insert("isAndroid", app.exec().contains("apkd-launcher"));

            tmp.append(map);
        }
    }

    // sort them by application name
    std::sort(tmp.begin(), tmp.end(), appNameLessThan);

    return tmp;
}

QVariantMap SettingsUi::getCurrentSettings()
{
    QVariantMap map;

    QSettings settings("harbour-tohkbd2", "tohkbd2");
    settings.beginGroup("generalsettings");

    map.insert("backlightTimeout", settings.value("backlightTimeout", 2000).toInt());

    map.insert("backlightLuxThreshold", settings.value("backlightLuxThreshold", 5).toInt());
    map.insert("keyRepeatDelay", settings.value("keyRepeatDelay", 250).toInt());
    map.insert("keyRepeatRate", settings.value("keyRepeatRate", 25).toInt());
    map.insert("backlightEnabled", settings.value("backlightEnabled", true).toBool());
    map.insert("forceLandscapeOrientation", settings.value("forceLandscapeOrientation", true).toBool());

    settings.endGroup();

    return map;
}

void SettingsUi::startApplication(QString appname)
{
    QProcess proc;
    proc.startDetached("/usr/bin/xdg-open" , QStringList() << appname);

    QThread::msleep(100);
}

QVariantList SettingsUi::getCurrentShortcuts()
{
    QVariantList tmp;
    QVariantMap map;

    QSettings settings("harbour-tohkbd2", "tohkbd2");
    settings.beginGroup("applicationshortcuts");

    for (int i = KEY_1 ; i<=KEY_EQUAL ; i++)
    {
        QString appPath = settings.value(QString("KEY_F%1").arg((i-KEY_1)+1), "none" ).toString();
        map.clear();
        map.insert("key", QString("F%1").arg((i-KEY_1)+1));
        map.insert("filePath", appPath);

        MDesktopEntry app(appPath);

        if (app.isValid())
            map.insert("name", app.name());
        else
            map.insert("name", "Not configured");

        if (app.icon().startsWith("icon-launcher-") || app.icon().startsWith("icon-l-") || app.icon().startsWith("icons-Applications"))
            map.insert("iconId", QString("image://theme/%1").arg(app.icon()));
        else if (app.icon().startsWith("/"))
            map.insert("iconId", QString("%1").arg(app.icon()));
        else
            map.insert("iconId", QString("/usr/share/icons/hicolor/86x86/apps/%1.png").arg(app.icon()));

        map.insert("isAndroid", app.exec().contains("apkd-launcher"));

        tmp.append(map);
    }

    settings.endGroup();

    return tmp;
}

void SettingsUi::setShortcut(QString key, QString appPath)
{
    qDebug() << "setting shortcut" << key << "to" << appPath;

    QDBusInterface tohkbd2daemon("com.kimmoli.tohkbd2", "/", "com.kimmoli.tohkbd2", QDBusConnection::systemBus());
    tohkbd2daemon.setTimeout(2000);
    QList<QVariant> args;
    args.append(key);
    args.append(appPath);
    tohkbd2daemon.callWithArgumentList(QDBus::AutoDetect, "setShortcut", args);

    emit shortcutsChanged();
}

void SettingsUi::setSettingInt(QString key, int value)
{
    qDebug() << "setting" << key << "to" << value;

    QDBusInterface tohkbd2daemon("com.kimmoli.tohkbd2", "/", "com.kimmoli.tohkbd2", QDBusConnection::systemBus());
    tohkbd2daemon.setTimeout(2000);
    QList<QVariant> args;
    args.append(key);
    args.append(value);
    tohkbd2daemon.callWithArgumentList(QDBus::AutoDetect, "setSettingInt", args);

    emit settingsChanged();
}

void SettingsUi::setShortcutsToDefault()
{
    qDebug() << "setting all shortcuts to default";

    QDBusInterface tohkbd2daemon("com.kimmoli.tohkbd2", "/", "com.kimmoli.tohkbd2", QDBusConnection::systemBus());
    tohkbd2daemon.setTimeout(2000);
    tohkbd2daemon.call(QDBus::AutoDetect, "setShortcutsToDefault");

    emit shortcutsChanged();
}

QString SettingsUi::readDaemonVersion()
{
    QDBusInterface getDaemonVersionCall("com.kimmoli.tohkbd2", "/", "com.kimmoli.tohkbd2", QDBusConnection::systemBus());
    getDaemonVersionCall.setTimeout(2000);

    QDBusMessage getDaemonVersionReply = getDaemonVersionCall.call(QDBus::AutoDetect, "getVersion");

    if (getDaemonVersionReply.type() == QDBusMessage::ErrorMessage)
    {
        qDebug() << "Error reading daemon version:" << getDaemonVersionReply.errorMessage();
        return QString("N/A");
    }

    QString daemonVersion = getDaemonVersionReply.arguments().at(0).toString();

    qDebug() << "Daemon version is" << daemonVersion;

    return daemonVersion;

}

QString SettingsUi::readUserDaemonVersion()
{
    QDBusInterface getUserDaemonVersionCall("com.kimmoli.tohkbd2user", "/", "com.kimmoli.tohkbd2user", QDBusConnection::sessionBus());
    getUserDaemonVersionCall.setTimeout(2000);

    QDBusMessage getUserDaemonVersionReply = getUserDaemonVersionCall.call(QDBus::AutoDetect, "getVersion");

    if (getUserDaemonVersionReply.type() == QDBusMessage::ErrorMessage)
    {
        qDebug() << "Error reading daemon version:" << getUserDaemonVersionReply.errorMessage();
        return QString("N/A");
    }

    QString userDaemonVersion = getUserDaemonVersionReply.arguments().at(0).toString();

    qDebug() << "User daemon version is" << userDaemonVersion;

    return userDaemonVersion;
}
