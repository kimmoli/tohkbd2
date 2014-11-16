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


int main(int argc, char *argv[])
{
    qmlRegisterType<SettingsUi>("harbour.tohkbd2.settingsui", 1, 0, "SettingsUi");

    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
    QScopedPointer<QQuickView> view(SailfishApp::createView());
    view->setSource(SailfishApp::pathTo("qml/tohkbd2-settingsui.qml"));
    view->show();

    return app->exec();
}

