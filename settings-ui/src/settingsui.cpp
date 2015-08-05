/*
    tohkbd2-settings-u, The Otherhalf Keyboard 2 settings UI
*/

#include "settingsui.h"
#include <QDir>
#include <QVariantMap>
#include <QVariantList>
#include <QThread>
#include <QSettings>
#include <QDebug>
#include <QtDBus/QtDBus>
#include <algorithm>
#include <mlite5/MDesktopEntry>
#include <linux/input.h>
#include "../../daemon/src/defaultSettings.h"
#include "settingsuiAdaptor.h"

static const char *SERVICE = SERVICE_NAME;
static const char *PATH = "/";


SettingsUi::SettingsUi(QObject *parent) :
    QObject(parent)
{
    m_dbusRegistered = false;

    new Tohkbd2settingsuiAdaptor(this);

    registerDBus();

    tohkbd2daemon = new ComKimmoliTohkbd2Interface("com.kimmoli.tohkbd2", "/", QDBusConnection::systemBus(), this);
    tohkbd2daemon->setTimeout(2000);
    tohkbd2user = new ComKimmoliTohkbd2userInterface("com.kimmoli.tohkbd2user", "/", QDBusConnection::sessionBus(), this);
    tohkbd2user->setTimeout(2000);

    connect(tohkbd2user, SIGNAL(physicalLayoutChanged(QString)), this, SLOT(handlePhysicalLayoutChange(QString)));

    layoutToLanguage.insert("cz" ,"Čeština");
    layoutToLanguage.insert("dk" ,"Dansk");
    layoutToLanguage.insert("de" ,"Deutsch");
    layoutToLanguage.insert("ee" ,"Eesti");
    layoutToLanguage.insert("us" ,"English");
    layoutToLanguage.insert("es" ,"Español");
    layoutToLanguage.insert("fr" ,"Français");
    layoutToLanguage.insert("it" ,"Italiano");
    layoutToLanguage.insert("nl" ,"Nederlands");
    layoutToLanguage.insert("no" ,"Norsk");
    layoutToLanguage.insert("pl" ,"Polski");
    layoutToLanguage.insert("pt" ,"Português");
    layoutToLanguage.insert("fi" ,"Suomi");
    layoutToLanguage.insert("se" ,"Svenska");
    layoutToLanguage.insert("tr" ,"Türkçe");
    layoutToLanguage.insert("kz" ,"Қазақ");
    layoutToLanguage.insert("ru" ,"Русский");

    emit versionChanged();
}

SettingsUi::~SettingsUi()
{
    if (m_dbusRegistered)
    {
        QDBusConnection connection = QDBusConnection::sessionBus();
        connection.unregisterObject(PATH);
        connection.unregisterService(SERVICE);

        printf("tohkbd2-settingsui: unregistered from dbus sessionBus\n");
    }
}

void SettingsUi::registerDBus()
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

        printf("tohkbd2-settingsui: succesfully registered to dbus sessionBus \"%s\"\n", SERVICE);
    }
}


QString SettingsUi::readVersion()
{
    return APPVERSION;
}

bool appNameLessThan(const QVariant &v1, const QVariant &v2)
{
    return v1.toMap()["name"].toString() < v2.toMap()["name"].toString();
}

QVariantList SettingsUi::getApplications()
{
    QVariantList tmp;
    QVariantMap map;
    QFileInfoList list;
    QDir dir;

    dir.setPath("/usr/share/applications/");
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    dir.setNameFilters(QStringList() << "*.desktop");
    dir.setSorting(QDir::Name);

    list = dir.entryInfoList();

    for (int i=0 ; i<list.size() ; i++)
    {
        MDesktopEntry app(list.at(i).absoluteFilePath());

        if (!app.hidden() && !app.icon().isEmpty() && !app.noDisplay() && !app.notShowIn().contains("X-Meego"))
        {
            map.clear();
            map.insert("filePath", list.at(i).absoluteFilePath());
            map.insert("name", app.name());
            if (app.icon().startsWith("icon-launcher-") || app.icon().startsWith("icon-l-") || app.icon().startsWith("icons-Applications"))
                map.insert("iconId", QString("image://theme/%1").arg(app.icon()));
            else if (app.icon().startsWith("/"))
                map.insert("iconId", QString("%1").arg(app.icon()));
            else
                map.insert("iconId", QString("/usr/share/icons/hicolor/86x86/apps/%1.png").arg(app.icon()));

            map.insert("isAndroid", app.exec().contains("apkd-launcher"));

            tmp.append(map);
        }
    }

    // sort them by application name
    std::sort(tmp.begin(), tmp.end(), appNameLessThan);

    return tmp;
}

QVariantMap SettingsUi::getCurrentSettings()
{
    QVariantMap map;

    QSettings settings("harbour-tohkbd2", "tohkbd2");
    settings.beginGroup("generalsettings");

    map.insert("backlightTimeout", settings.value("backlightTimeout", BACKLIGHT_TIMEOUT).toInt());
    map.insert("backlightLuxThreshold", settings.value("backlightLuxThreshold", BACKLIGHT_LUXTHRESHOLD).toInt());
    map.insert("keyRepeatDelay", settings.value("keyRepeatDelay", KEYREPEAT_DELAY).toInt());
    map.insert("keyRepeatRate", settings.value("keyRepeatRate", KEYREPEAT_RATE).toInt());
    map.insert("backlightEnabled", settings.value("backlightEnabled", BACKLIGHT_ENABLED).toBool());
    map.insert("forceLandscapeOrientation", settings.value("forceLandscapeOrientation", FORCE_LANDSCAPE_ORIENTATION).toBool());
    map.insert("forceBacklightOn", settings.value("forceBacklightOn", FORCE_BACKLIGHT_ON).toBool());
    map.insert("modifierShiftMode", settings.value("modifierShiftMode", MODIFIER_SHIFT_MODE).toString());
    map.insert("modifierCtrlMode", settings.value("modifierCtrlMode", MODIFIER_CTRL_MODE).toString());
    map.insert("modifierAltMode", settings.value("modifierAltMode", MODIFIER_ALT_MODE).toString());
    map.insert("modifierSymMode", settings.value("modifierSymMode", MODIFIER_SYM_MODE).toString());
    settings.endGroup();

    settings.beginGroup("debug");
    map.insert("verboseMode", settings.value("verboseMode", VERBOSE_MODE_ENABLED).toBool());
    settings.endGroup();

    QString layout = QString(tohkbd2user->getActivePhysicalLayout());

    map.insert("physicalLayout", layoutToLanguage.value(layout));

    return map;
}

QVariantList SettingsUi::getCurrentShortcuts()
{
    QVariantList tmp;
    QVariantMap map;

    QSettings settings("harbour-tohkbd2", "tohkbd2");
    settings.beginGroup("applicationshortcuts");

    for (int i = KEY_1 ; i<=KEY_EQUAL ; i++)
    {
        QString appPath = settings.value(QString("KEY_F%1").arg((i-KEY_1)+1), "" ).toString();
        map.clear();
        map.insert("key", QString("F%1").arg((i-KEY_1)+1));
        map.insert("filePath", appPath);

        MDesktopEntry app(appPath);

        if (app.isValid())
        {
            map.insert("name", app.name());

            if (app.icon().startsWith("icon-launcher-") || app.icon().startsWith("icon-l-") || app.icon().startsWith("icons-Applications"))
                map.insert("iconId", QString("image://theme/%1").arg(app.icon()));
            else if (app.icon().startsWith("/"))
                map.insert("iconId", QString("%1").arg(app.icon()));
            else
                map.insert("iconId", QString("/usr/share/icons/hicolor/86x86/apps/%1.png").arg(app.icon()));

            map.insert("isAndroid", app.exec().contains("apkd-launcher"));
        }
        else
        {
            //: Label shown instead of application name if Fn is not configured
            //% "Not configured"
            map.insert("name", qtTrId("shortcut-not-configured"));
            map.insert("iconId", QString());
            map.insert("isAndroid", false);
        }

        tmp.append(map);
    }

    settings.endGroup();

    return tmp;
}

void SettingsUi::setShortcut(QString key, QString appPath)
{
    qDebug() << "setting shortcut" << key << "to" << appPath;

    tohkbd2daemon->setShortcut(key, appPath);

    QThread::msleep(200);

    emit shortcutsChanged();
}

void SettingsUi::setSettingInt(QString key, int value)
{
    qDebug() << "setting" << key << "to" << value;

    tohkbd2daemon->setSettingInt(key, value);

    QThread::msleep(200);

    emit settingsChanged();
}

void SettingsUi::setSettingString(QString key, QString value)
{
    qDebug() << "setting" << key << "to" << value;

    tohkbd2daemon->setSettingString(key, value);

    QThread::msleep(200);

    emit settingsChanged();
}

void SettingsUi::setShortcutsToDefault()
{
    qDebug() << "setting all shortcuts to default";

    tohkbd2daemon->setShortcutsToDefault();

    QThread::msleep(200);

    emit shortcutsChanged();
}

void SettingsUi::setSettingsToDefault()
{
    setSettingInt("backlightTimeout", BACKLIGHT_TIMEOUT);
    setSettingInt("backlightLuxThreshold", BACKLIGHT_LUXTHRESHOLD);
    setSettingInt("keyRepeatDelay", KEYREPEAT_DELAY);
    setSettingInt("keyRepeatRate", KEYREPEAT_RATE);
    setSettingInt("backlightEnabled", BACKLIGHT_ENABLED ? 1 : 0);
    setSettingInt("forceLandscapeOrientation", FORCE_LANDSCAPE_ORIENTATION ? 1 : 0);
    setSettingInt("forceBacklightOn", FORCE_BACKLIGHT_ON ? 1 : 0);
    setSettingString("modifierShiftMode", MODIFIER_SHIFT_MODE);
    setSettingString("modifierCtrlMode", MODIFIER_CTRL_MODE);
    setSettingString("modifierAltMode", MODIFIER_ALT_MODE);
    setSettingString("modifierSymMode", MODIFIER_SYM_MODE);
    setSettingInt("verboseMode", VERBOSE_MODE_ENABLED ? 1 : 0);

    QThread::msleep(200);

    emit settingsChanged();
}

QString SettingsUi::readDaemonVersion()
{
    QString daemonVersion = tohkbd2daemon->getVersion();

    if (tohkbd2daemon->lastError().type() != QDBusError::NoError)
    {
        qDebug() << "Error getting daemon version " << QDBusError::errorString(tohkbd2daemon->lastError().type());
        daemonVersion = QString("N/A");
    }
    else
    {
        qDebug() << "Daemon version is" << daemonVersion;
    }

    return daemonVersion;
}

QString SettingsUi::readUserDaemonVersion()
{
    QString userDaemonVersion = tohkbd2user->getVersion();

    if (tohkbd2user->lastError().type() != QDBusError::NoError)
    {
        qDebug() << "Error getting user daemon version " << QDBusError::errorString(tohkbd2user->lastError().type());
        userDaemonVersion = QString("N/A");
    }
    else
    {
        qDebug() << "User daemon version is" << userDaemonVersion;
    }

    return userDaemonVersion;
}

QString SettingsUi::readSailfishVersion()
{
    QString version = "N/A";

    QFile inputFile( "/etc/sailfish-release" );

    if ( inputFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
       QTextStream in( &inputFile );

       while (not in.atEnd())
       {
           QString line = in.readLine();
           if (line.startsWith("VERSION_ID="))
           {
               version = line.split('=').at(1);
               break;
           }
       }
       inputFile.close();
    }
    qDebug() << "Sailfish version is" << version;

    return version;
}

void SettingsUi::handlePhysicalLayoutChange(QString layout)
{
    Q_UNUSED(layout);

    emit settingsChanged();
}

void SettingsUi::forceKeymapReload()
{
    tohkbd2daemon->forceKeymapReload(QString());
}

void SettingsUi::restoreOriginalKeymaps()
{
    tohkbd2user->installKeymaps(true);
}

QVariantList SettingsUi::getCurrentLayouts()
{
    QVariantList tmp;
    QVariantMap map;

    QFileInfoList list;
    QDir dir;
    QStringList keymaps;

    /* Get list of .tohkbdmap files in config */
    dir.setPath(QString(tohkbd2user->getPathTo("keymaplocation")));
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    dir.setNameFilters(QStringList() << "*.tohkbdmap");

    list = dir.entryInfoList();
    for (int i=0 ; i<list.size() ; i++)
    {
        keymaps << list.at(i).fileName().split(".").at(0);
    }

    QMap<QString, QString>::iterator layout;
    for (layout = layoutToLanguage.begin(); layout != layoutToLanguage.end(); ++layout)
    {
        /* key = us , name = English, supported = true */
        map.clear();
        map.insert("key", layout.key());
        map.insert("name", layout.value());
        map.insert("supported", keymaps.contains(layout.key()));
        tmp.append(map);
    }

    std::sort(tmp.begin(), tmp.end(), appNameLessThan);

    return tmp;
}
