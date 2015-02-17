#include "taskswitcher.h"

Taskswitcher::Taskswitcher(QObject *parent) :
    QObject(parent)
{
}

void Taskswitcher::launchApplication(const QString &app)
{
    QProcess proc;
    proc.startDetached("/usr/bin/xdg-open" , QStringList() << app);

    QThread::msleep(100);
}

/* Getting shortcuts defined in TOHKBD2, F1 through F8 */
QVariantList Taskswitcher::getCurrentShortcuts()
{
    QVariantList tmp;
    QVariantMap map;

    QStringList apps;
    apps << "/usr/share/applications/sailfish-browser.desktop";
    apps << "/usr/share/applications/fingerterm.desktop";
    apps << "/usr/share/applications/voicecall-ui.desktop";
    apps << "/usr/share/applications/sailfish-maps.desktop";
    apps << "/usr/share/applications/jolla-camera.desktop";
    apps << "/usr/share/applications/jolla-gallery.desktop";
    apps << "/usr/share/applications/jolla-clock.desktop";
    apps << "/usr/share/applications/jolla-email.desktop";


    for (int i = 0 ; i<apps.count() ; i++)
    {
        QString appPath = apps.at(i);

        map.clear();
        map.insert("key", QString("F%1").arg(i));
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

    return tmp;
}
