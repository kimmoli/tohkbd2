#include <stdio.h>
#include <sailfishapp.h>
#include <QHostAddress>
#include <QtSystemInfo/QDeviceInfo>

#include "userdaemon.h"

static const char *SERVICE = SERVICE_NAME;
static const char *PATH = "/";

UserDaemon::UserDaemon(QObject *parent) :
    QObject(parent)
{
    m_dbusRegistered = false;
    m_launchPending = false;

    installKeymaps(false);

    physicalLayout = new MGConfItem("/desktop/lipstick-jolla-home/layout");
    connect(physicalLayout, SIGNAL(valueChanged()), this, SLOT(handlePhysicalLayout()));

    /* Remove tohkbd from enabled layouts if vkb is hidden by maliit.
     * The related PR is merged in maliit-framework and should be in 2.0 */
    if (checkSailfishVersion("2.0.0.0"))
    {
        QString tohlayout("harbour-tohkbd2.qml");
        MGConfItem el("/sailfish/text_input/enabled_layouts");
        QStringList list = el.value().toStringList();
        if (list.contains(tohlayout))
        {
            list.removeAll(tohlayout);
            el.set(list);
        }
    }
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
        
        QString tohlayout("harbour-tohkbd2.qml");
        MGConfItem el("/sailfish/text_input/enabled_layouts");
        QStringList list = el.value().toStringList();
        if (value.compare(tohlayout) == 0) {
            if (!list.contains(tohlayout)) {
                list.append(tohlayout);
                el.set(list);
            }
        }
        else {
            if (list.contains(tohlayout)) {
                list.removeAll(tohlayout);
                el.set(list);
            }
        }
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
    notif.setCategory("x-harbour.tohkbd2");
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

QString UserDaemon::getPathTo(const QString &filename)
{
    if (filename == "keymaplocation")
    {
        return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + KEYMAP_FOLDER;
    }

    return SailfishApp::pathTo(filename).toLocalFile();
}

void UserDaemon::showUnsupportedLayoutNotification()
{
    //: Notification shown when a physical layout is not supported or the config file has an error. Notification text will scroll.
    //% "The selected physical layout is not supported by TOHKBD2. Config file can also be invalid or missing."
    showNotification(qtTrId("layout-unsupported"));
}

void UserDaemon::installKeymaps(const bool &overwrite)
{
    QDir keymapfolder(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + KEYMAP_FOLDER);
    keymapfolder.mkpath(".");

    QDir keymapRes(":/layouts/");
    QFileInfoList list = keymapRes.entryInfoList();

    int i;
    for (i=0 ; i < list.size() ; i++)
    {
        QString from = list.at(i).absoluteFilePath();
        QString to = keymapfolder.path() + "/" + from.split("/").last();

        QFileInfo toFile(to);

        if(!toFile.exists() || overwrite)
        {
            QFile newToFile(to);
            QResource res(from);

            if (newToFile.open(QIODevice::WriteOnly) && res.isValid())
            {
                qint64 ws;
                if (res.isCompressed())
                    ws = newToFile.write( qUncompress(res.data(), res.size()));
                else
                    ws = newToFile.write( (char *)res.data());

                newToFile.close();
                printf("tohkbd2-user: Wrote %s (%lld bytes) to %s\n", qPrintable(from), ws, qPrintable(to));
            }
            else
            {
                printf("tohkbd2-user: Failed to write %s\n", qPrintable(to));
            }
        }
    }
}

void UserDaemon::setKeymapLayout(const QString &value)
{
    MGConfItem keymapLayout("/desktop/lipstick-jolla-home/layout");
    keymapLayout.set(value);
}

void UserDaemon::setKeymapVariant(const QString &value)
{
    MGConfItem keymapVariant("/desktop/lipstick-jolla-home/variant");
    keymapVariant.set(value);
}

/*
 * Check that Sailfish version is at least required version
 */
bool UserDaemon::checkSailfishVersion(QString versionToCompare)
{
    QDeviceInfo deviceInfo;
    QString sailfishVersion = deviceInfo.version(QDeviceInfo::Os);

    return (QHostAddress(sailfishVersion).toIPv4Address()
            >= QHostAddress(versionToCompare).toIPv4Address());
}
