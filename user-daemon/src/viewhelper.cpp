#include "viewhelper.h"

#include <stdio.h>

#include <QGuiApplication>
#include <QDir>
#include <qpa/qplatformnativeinterface.h>
#include <QFileInfo>

ViewHelper::ViewHelper(QQuickView *parent) :
    QObject(parent),
    view(parent)
{
    m_currentApp = 0;
    m_numberOfApps = 0;
    m_visible = false;

    apps.clear();
    appsDesktopFiles.clear();

    emit currentAppChanged();
    emit numberOfAppsChanged();
    emit visibleChanged();
}

void ViewHelper::detachWindow()
{
    view->close();
    view->create();

    QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
    native->setWindowProperty(view->handle(), QLatin1String("CATEGORY"), "notification");
    setDefaultRegion();
}

void ViewHelper::setMouseRegion(const QRegion &region)
{
    QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
    native->setWindowProperty(view->handle(), QLatin1String("MOUSE_REGION"), region);
}

void ViewHelper::setTouchRegion(const QRect &rect)
{
    setMouseRegion(QRegion(rect));
}

void ViewHelper::setDefaultRegion()
{
    setMouseRegion(QRegion( (540-240)/2, (960-480)/2, 240, 480));
}

void ViewHelper::hideWindow()
{
    m_visible = false;
    emit visibleChanged();
    launchApplication(m_currentApp);
}

void ViewHelper::showWindow()
{
    printf("tohkbd2-user: showing taskswitcher\n");

    QProcess ps;
    ps.start("ps", QStringList() << "ax" << "-o" << "cmd=");
    ps.waitForFinished();
    QStringList pr = QString(ps.readAllStandardOutput()).split("\n");

    QStringList cmd;
    /* TODO: Add support for android apps */
    for (int i=0 ; i<pr.count() ; i++)
    {
        if ((pr.at(i).contains("invoker") && pr.at(i).contains("silica")) ||
                pr.at(i).contains("jolla-") ||
                pr.at(i).contains("sailfish-") ||
                (pr.at(i).contains("invoker") && pr.at(i).contains("fingerterm")))
        {
            cmd << pr.at(i);
        }
    }

    QStringList exec;
    for (int i=0 ; i<cmd.count() ; i++)
    {
        QStringList tmp = cmd.at(i).split(" ");
        for (int a=0 ; a<tmp.count() ; a++)
        {
            if (!tmp.at(a).startsWith("-") && !tmp.at(a).contains("invoker") && !tmp.at(a).isEmpty())
            {
                QFileInfo fi(QDir("/usr/bin"), tmp.at(a));
                if (fi.exists() && fi.isExecutable())
                {
                    exec << tmp.at(a);
                }
            }
        }
    }

    exec.removeDuplicates();

    QVariantMap map;

    QFileInfoList list;
    QDir dir;
    QStringList desktops;

    apps.clear();
    appsDesktopFiles.clear();

    dir.setPath("/usr/share/applications/");
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    dir.setNameFilters(QStringList() << "*.desktop");
    dir.setSorting(QDir::Name);

    list = dir.entryInfoList();

    for (int i=0 ; i<list.size() ; i++)
    {
        desktops << list.at(i).absoluteFilePath();
    }

    for (int i = 0 ; i < desktops.count() ; i++)
    {
        MDesktopEntry app(desktops.at(i));

        for (int m = 0 ; m < exec.count() ; m++)
        {
            if (app.exec().contains(exec.at(m)) && app.isValid() && !app.hidden() && !app.noDisplay() && desktops.at(i).contains(exec.at(m).split("/").last()))
            {
                map.clear();
                map.insert("name", app.name());
                if (app.icon().startsWith("icon-launcher-") || app.icon().startsWith("icon-l-") || app.icon().startsWith("icons-Applications"))
                    map.insert("iconId", QString("image://theme/%1").arg(app.icon()));
                else if (app.icon().startsWith("/"))
                    map.insert("iconId", QString("%1").arg(app.icon()));
                else
                    map.insert("iconId", QString("/usr/share/icons/hicolor/86x86/apps/%1.png").arg(app.icon()));

                apps.append(map);
                appsDesktopFiles.append(desktops.at(i));

                printf("tohkbd2-user: %s\n", qPrintable(desktops.at(i)));

                exec.removeAt(m);

                if (apps.count() > 15)
                    break;
            }
        }
        if (apps.count() > 15)
            break;
    }

    m_numberOfApps = apps.count();

    emit numberOfAppsChanged();

    if (m_numberOfApps > 1)
    {
        view->showFullScreen();
        m_visible = true;
        emit visibleChanged();
    }
}

void ViewHelper::nextApp()
{
    if (m_numberOfApps > 0)
        m_currentApp = (m_currentApp+1) % m_numberOfApps;

    emit currentAppChanged();
}

int ViewHelper::getCurrentApp()
{
    return m_currentApp;
}

int ViewHelper::getNumberOfApps()
{
    return m_numberOfApps;
}

bool ViewHelper::getVisible()
{
    return m_visible;
}

/* Testing */
void ViewHelper::setNumberOfApps(int n)
{
    m_numberOfApps = n;

    if (m_currentApp > m_numberOfApps)
    {
        m_currentApp = m_numberOfApps;
        emit currentAppChanged();
    }

    emit numberOfAppsChanged();

}

void ViewHelper::setCurrentApp(int n)
{
    m_currentApp = n;
    emit currentAppChanged();
}

void ViewHelper::launchApplication(int n)
{
    printf("tohkbd2-user: Starting %s\n", qPrintable(appsDesktopFiles.at(n)));

    view->hide();

    emit _launchApplication(appsDesktopFiles.at(n));
}

QVariantList ViewHelper::getCurrentApps()
{
    return apps;
}
