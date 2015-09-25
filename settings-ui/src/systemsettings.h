#ifndef SYSTEMSETTINGS_H
#define SYSTEMSETTINGS_H

#include <QObject>
#include <QVariantMap>

#include "../dbus/src/mceInterface.h"

#define MCE_GCONF_KBD_SLIDE_INHIBIT     "/system/osso/dsm/display/kbd_slide_inhibit_blank_mode"
#define MCE_GCONF_TK_KBD_OPEN_ACTIONS   "/system/osso/dsm/locks/keyboard_open_actions"
#define MCE_GCONF_TK_KBD_CLOSE_ACTIONS  "/system/osso/dsm/locks/keyboard_close_actions"
#define MCE_GCONF_TK_KBD_OPEN_TRIGGER   "/system/osso/dsm/locks/keyboard_open_trigger"
#define MCE_GCONF_TK_KBD_CLOSE_TRIGGER  "/system/osso/dsm/locks/keyboard_close_trigger"

/*
    KBD_SLIDE_INHIBIT_OFF                = 0
    KBD_SLIDE_INHIBIT_STAY_ON_WHEN_OPEN  = 1
    KBD_SLIDE_INHIBIT_STAY_DIM_WHEN_OPEN = 2

    LID_OPEN_ACTION_DISABLED = 0
    LID_OPEN_ACTION_UNBLANK  = 1
    LID_OPEN_ACTION_TKUNLOCK = 2

    LID_CLOSE_ACTION_DISABLED = 0
    LID_CLOSE_ACTION_BLANK    = 1
    LID_CLOSE_ACTION_TKLOCK   = 2
*/

class SystemSettings : public QObject
{
    Q_OBJECT
public:
    explicit SystemSettings(QObject *parent = 0);

    Q_INVOKABLE QVariant value(const QString &path);
    Q_INVOKABLE void setValue(const QString &path, const QVariant &value);

signals:
    void settingChanged(const QString &path, const QVariant &value);

private slots:
    void configChange(const QString &path, const QDBusVariant &value);

private:
    ComNokiaMceRequestInterface *mceRequest;
    ComNokiaMceSignalInterface *mceSignal;
    QVariantMap settings;
    static QStringList settingPaths;
};

#endif // SYSTEMSETTINGS_H
