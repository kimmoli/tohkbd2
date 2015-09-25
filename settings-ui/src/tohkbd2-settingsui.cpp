/*
    tohkbd2-settings-u, The Otherhalf Keyboard 2 settings UI
*/

#include <sailfishapp.h>
#include <QtQml>
#include <QScopedPointer>
#include <QQuickView>
#include <QQmlEngine>
#include <QGuiApplication>
#include <QQmlContext>
#include <QCoreApplication>
#include "settingsui.h"
#include "IconProvider.h"
#include "consolemodel.h"
#include "systemsettings.h"

int main(int argc, char *argv[])
{
    qmlRegisterType<SettingsUi>("harbour.tohkbd2.settingsui", 1, 0, "SettingsUi");
    qmlRegisterType<ConsoleModel>("harbour.tohkbd2.ConsoleModel", 1, 0, "ConsoleModel");
    qmlRegisterType<SystemSettings>("harbour.tohkbd2.systemsettings", 1, 0, "SystemSettings");

    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));

    QTranslator engineeringTranslator;
    engineeringTranslator.load("engineering_en", SailfishApp::pathTo("i18n").toLocalFile());
    app->installTranslator(&engineeringTranslator);

    QTranslator translator;
    translator.load(QLocale::system().name(), SailfishApp::pathTo("i18n").toLocalFile());
    app->installTranslator(&translator);

    QScopedPointer<QQuickView> view(SailfishApp::createView());

    QQmlEngine *engine = view->engine();
    engine->addImageProvider(QLatin1String("tohkbd2"), new IconProvider);

    QString viewMode;

    if (argc > 1)
        viewMode = QString(argv[1]);

    if (!viewMode.startsWith("--"))
        viewMode = QString();

    view->rootContext()->setContextProperty("viewMode", viewMode);

    view->setSource(SailfishApp::pathTo("qml/tohkbd2-settingsui.qml"));
    view->show();

    QObject::connect(engine, SIGNAL(quit()), app.data(), SLOT(quit()));

    return app->exec();
}

