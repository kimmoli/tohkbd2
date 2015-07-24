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

    physicalLayout = new MGConfItem("/desktop/lipstick-jolla-home/layout");
    connect(physicalLayout, SIGNAL(valueChanged()), this, SLOT(handlePhysicalLayout()));
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
    {
        //: Notification shown when keyboard is connected
        //% "Keyboard connected"
        showNotification(qtTrId("keyb-connected"));
    }
    else
    {
        //: Notification shown when keyboard is removed
        //% "Keyboard removed"
        showNotification(qtTrId("keyb-removed"));
    }
}

QString UserDaemon::getVersion()
{
    return QString(APPVERSION);
}

void UserDaemon::launchSuccess(const QString &appName)
{
    if (m_launchPending)
    {
        //: Notification shown when application is started by pressing shortcut key
        //% "Starting %1..."
        showNotification(qtTrId("starting-app").arg(appName));
    }

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
    Notification notif;

    notif.setPreviewBody(text);
    notif.setHintValue("x-nemo-preview-icon", SailfishApp::pathTo("/icon-system-keyboard.png").toLocalFile());
    notif.publish();
}

void UserDaemon::actionWithRemorse(const QString &action)
{
    printf("tohkbd2-user: requested %s.\n", qPrintable(action));

    emit _requestActionWithRemorse(action);
}

void UserDaemon::handlePhysicalLayout()
{
    emit physicalLayoutChanged(getActivePhysicalLayout());
}

QString UserDaemon::getActivePhysicalLayout()
{
    return physicalLayout->value().toString();
}

QString UserDaemon::getPathTo(QString filename)
{
    if (filename == "keymaplocation")
    {
        return QDir::homePath() + KEYMAP_FOLDER;
    }

    return SailfishApp::pathTo(filename).toLocalFile();
}
