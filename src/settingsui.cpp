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
            map.insert("filePath", list.at(i).absoluteFilePath());
            map.insert("name", app.name());
            if (app.icon().startsWith("icon-launcher-") || app.icon().startsWith("icon-l-") || app.icon().startsWith("icons-Applications"))
                map.insert("iconId", QString("image://theme/%1").arg(app.icon()));
            else if (app.icon().startsWith("/"))
                map.insert("iconId", QString("%1").arg(app.icon()));
            else
                map.insert("iconId", QString("/usr/share/icons/hicolor/86x86/apps/%1.png").arg(app.icon()));
            tmp.append(map);
        }
    }
    return tmp;
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

        tmp.append(map);
    }

    settings.endGroup();

    return tmp;
}
