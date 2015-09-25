#include "systemsettings.h"

#include <mce/dbus-names.h>
#include <mce/mode-names.h>

QStringList SystemSettings::settingPaths = QStringList()
        << MCE_GCONF_KBD_SLIDE_INHIBIT
        << MCE_GCONF_TK_KBD_OPEN_ACTIONS
        << MCE_GCONF_TK_KBD_CLOSE_ACTIONS
        << MCE_GCONF_TK_KBD_OPEN_TRIGGER
        << MCE_GCONF_TK_KBD_CLOSE_TRIGGER;

SystemSettings::SystemSettings(QObject *parent) :
    QObject(parent)
{

    mceRequest = new ComNokiaMceRequestInterface(MCE_SERVICE, MCE_REQUEST_PATH, QDBusConnection::systemBus(), this);

    foreach (const QString &path, settingPaths)
    {
        QDBusPendingReply<QDBusVariant> result = mceRequest->get_config(QDBusObjectPath(path));
        result.waitForFinished();
        qDebug() << path << result.value().variant();
        settings.insert(path, result.value().variant());
        emit settingChanged(path, result.value().variant());
    }

    mceSignal = new ComNokiaMceSignalInterface(MCE_SERVICE, MCE_SIGNAL_PATH, QDBusConnection::systemBus(), this);
    connect(mceSignal, SIGNAL(config_change_ind(QString, QDBusVariant)), this, SLOT(configChange(QString, QDBusVariant)));
}

QVariant SystemSettings::value(const QString &path)
{
    return settings.value(path);
}

void SystemSettings::setValue(const QString &path, const QVariant &value)
{
    settings.insert(path, value);
    mceRequest->set_config(QDBusObjectPath(path), QDBusVariant(value));
}

void SystemSettings::configChange(const QString &path, const QDBusVariant &value)
{
    qDebug() << "config changed" << path << value.variant();
    settings.insert(path, value.variant());
    emit settingChanged(path, value.variant());
}
