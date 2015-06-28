#include <stdio.h>
#include <sailfishapp.h>
#include "userdaemon.h"

static const char *SERVICE = SERVICE_NAME;
static const char *PATH = "/";

UserDaemon::UserDaemon(QObject *parent) :
    QObject(parent)
{
    m_dbusRegistered = false;
    m_launchPending = false;
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

void UserDaemon::setOrientationLock(const QString &value)
{
    if (value == "dynamic" || value == "landscape" || value == "portrait")
    {
        printf("tohkbd2-user: setting orientation lock to \"%s\"\n", qPrintable(value));

        MGConfItem ci("/lipstick/orientationLock");
        ci.set(value);
    }
    else
    {
        printf("tohkbd2-user: error: orientation lock can be set only to dynamic, landscape or portrait.\n");
    }
}

QString UserDaemon::getOrientationLock()
{
    MGConfItem ci("/lipstick/orientationLock");

    QString orientation = ci.value().toString();

    /* Assume orientation to be "dynamic" if returns empty */

    if (orientation.isEmpty())
        orientation = "dynamic";

    printf("tohkbd2-user: orientation lock is \"%s\"\n", qPrintable(orientation));

    return orientation;
}

void UserDaemon::launchApplication(const QString &desktopFilename)
{
    m_launchPending = true;
    emit _lauchApplication(desktopFilename);
}

void UserDaemon::showKeyboardConnectionNotification(const bool &connected)
{
    if (connected)
        showNotification(tr("Keyboard connected"));
    else
        showNotification(tr("Keyboard removed"));
}

QString UserDaemon::getVersion()
{
    return QString(APPVERSION);
}

void UserDaemon::launchSuccess(const QString &appName)
{
    if (m_launchPending)
        showNotification(tr("Starting %1...").arg(appName));

    m_launchPending = false;
}

void UserDaemon::launchFailed()
{
    m_launchPending = false;
}

/* show notification
 */
void UserDaemon::showNotification(const QString &text)
{
    MNotification notification(MNotification::DeviceEvent, "", text);
    notification.setImage(SailfishApp::pathTo("/icon-system-keyboard.png").toLocalFile());
    notification.publish();
}

void UserDaemon::resetWithRemorse()
{
    printf("tohkbd2-user: reset requested\n");

    emit _requestReboot();
}
