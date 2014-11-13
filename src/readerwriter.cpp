#include <stdio.h>
#include "readerwriter.h"

static const char *SERVICE = SERVICE_NAME;
static const char *PATH = "/";

ReaderWriter::ReaderWriter(QObject *parent) :
    QObject(parent)
{
    m_dbusRegistered = false;
}

ReaderWriter::~ReaderWriter()
{
    if (m_dbusRegistered)
    {
        QDBusConnection connection = QDBusConnection::sessionBus();
        connection.unregisterObject(PATH);
        connection.unregisterService(SERVICE);

        printf("tohkbd2user: unregistered from dbus sessionBus\n");
    }
}

void ReaderWriter::registerDBus()
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

        printf("tohkbd2user: succesfully registered to dbus sessionBus \"%s\"\n", SERVICE);
    }
}

void ReaderWriter::quit()
{
    printf("tohkbd2user: quit requested from dbus\n");
    QCoreApplication::quit();
}

void ReaderWriter::setActiveLayout(const QString &value)
{
    if (value.contains("qml"))
    {
        printf("tohkbd2user: setting active layout to \"%s\"\n", qPrintable(value));

        MGConfItem ci("/sailfish/text_input/active_layout");
        ci.set(value);
    }
    else
    {
        printf("tohkbd2user: value \"%s\" does not look like layout, refused to write\n", qPrintable(value));
    }
}

QString ReaderWriter::getActiveLayout()
{
    MGConfItem ci("/sailfish/text_input/active_layout");

    printf("tohkbd2user: active layout is \"%s\"\n", qPrintable(ci.value().toString()));

    return ci.value().toString();
}

