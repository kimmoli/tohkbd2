#include <stdio.h>
#include "userdaemon.h"

static const char *SERVICE = SERVICE_NAME;
static const char *PATH = "/";

UserDaemon::UserDaemon(QObject *parent) :
    QObject(parent)
{
    m_dbusRegistered = false;
}

UserDaemon::~UserDaemon()
{
    if (m_dbusRegistered)
    {
        QDBusConnection connection = QDBusConnection::sessionBus();
        connection.unregisterObject(PATH);
        connection.unregisterService(SERVICE);

        printf("tohkbd2-user: unregistered from dbus sessionBus\n");
    }
}

void UserDaemon::registerDBus()
{
    if (!m_dbusRegistered)
    {
        // DBus
        QDBusConnection connection = QDBusConnection::sessionBus();
        if (!connection.registerService(SERVICE))
        {
            QCoreApplication::quit();
            return;
        }

        if (!connection.registerObject(PATH, this))
        {
            QCoreApplication::quit();
            return;
        }
        m_dbusRegistered = true;

        printf("tohkbd2-user: succesfully registered to dbus sessionBus \"%s\"\n", SERVICE);
    }
}

void UserDaemon::quit()
{
    printf("tohkbd2-user: quit requested from dbus\n");
    QCoreApplication::quit();
}

void UserDaemon::setActiveLayout(const QString &value)
{
    if (value.contains("qml"))
    {
        printf("tohkbd2-user: setting active layout to \"%s\"\n", qPrintable(value));

        MGConfItem ci("/sailfish/text_input/active_layout");
        ci.set(value);
    }
    else
    {
        printf("tohkbd2-user: value \"%s\" does not look like layout, refused to write\n", qPrintable(value));
    }
}

QString UserDaemon::getActiveLayout()
{
    MGConfItem ci("/sailfish/text_input/active_layout");

    printf("tohkbd2-user: active layout is \"%s\"\n", qPrintable(ci.value().toString()));

    return ci.value().toString();
}

void UserDaemon::launchApplication(const QString &desktopFilename)
{
    MDesktopEntry app(desktopFilename);

    printf("tohkbd2-user: starting \"%s\"\n" ,qPrintable(app.name()));

    showNotification(tr("Starting %1...").arg(app.name()));

    QProcess proc;
    proc.startDetached("/usr/bin/xdg-open" , QStringList() << desktopFilename);

    QThread::msleep(100);
}

void UserDaemon::showKeyboardConnectionNotification(const bool &connected)
{
    if (connected)
        showNotification(tr("Keyboard connected"));
    else
        showNotification(tr("Keyboard removed"));
}


/******** PRIV *********/

/* show notification
 */
void UserDaemon::showNotification(QString text)
{
    MNotification notification(MNotification::DeviceEvent, "", text);
    notification.setImage("/usr/share/harbour-tohkbd2/icon-system-keyboard.png");
    notification.publish();
}
