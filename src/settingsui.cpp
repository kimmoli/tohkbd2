/*
    tohkbd2-settings-u, The Otherhalf Keyboard 2 settings UI
*/

#include "settingsui.h"
#include <QDir>
#include <QVariantMap>
#include <QVariantList>
#include <QProcess>
#include <QThread>
#include <mlite5/MDesktopEntry>

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

    QDir dir;
    dir.setPath("/usr/share/applications/");
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    dir.setNameFilters(QStringList() << "*.desktop");

    QFileInfoList list = dir.entryInfoList();

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
