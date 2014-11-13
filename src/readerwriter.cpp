#include "readerwriter.h"

static const char *SERVICE = SERVICE_NAME;
static const char *PATH = "/";

ReaderWriter::ReaderWriter(QObject *parent) :
    QObject(parent)
{
}

ReaderWriter::~ReaderWriter()
{
    if (m_dbusRegistered)
    {
        QDBusConnection connection = QDBusConnection::sessionBus();
        connection.unregisterObject(PATH);
        connection.unregisterService(SERVICE);
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
    }
}

void ReaderWriter::setActiveLayout(const QString &value)
{
    if (value.contains("qml"))
    {
        printf("setting active layout to \"%s\"\n", qPrintable(value));

        MGConfItem ci("/sailfish/text_input/active_layout");
        ci.set(value);
    }
    else
    {
        printf("value \"%s\" does not look like layout\n", qPrintable(value));
    }
}

QString ReaderWriter::getActiveLayout()
{
    MGConfItem ci("/sailfish/text_input/active_layout");

    printf("active layout is \"%s\"\n", qPrintable(ci.value().toString()));

    return ci.value().toString();
}

