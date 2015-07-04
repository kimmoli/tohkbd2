/*
    tohkbd2-settings-u, The Otherhalf Keyboard 2 settings UI
*/


#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

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


int main(int argc, char *argv[])
{
    qmlRegisterType<SettingsUi>("harbour.tohkbd2.settingsui", 1, 0, "SettingsUi");

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

    view->setSource(SailfishApp::pathTo("qml/tohkbd2-settingsui.qml"));
    view->show();

    return app->exec();
}

