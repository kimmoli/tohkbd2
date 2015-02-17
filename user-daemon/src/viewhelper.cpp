#include "viewhelper.h"

#include <QGuiApplication>
#include <qpa/qplatformnativeinterface.h>
#include <QDebug>

ViewHelper::ViewHelper(QQuickView *parent) :
    QObject(parent),
    view(parent)
{
}

void ViewHelper::detachWindow()
{
    view->close();
    view->create();

    QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
    native->setWindowProperty(view->handle(), QLatin1String("CATEGORY"), "notification");
    setDefaultRegion();

    //view->showFullScreen();
}

void ViewHelper::setMouseRegion(const QRegion &region)
{
    QPlatformNativeInterface *native = QGuiApplication::platformNativeInterface();
    native->setWindowProperty(view->handle(), QLatin1String("MOUSE_REGION"), region);
}

//void ViewHelper::setTouchRegion(const QRect &rect)
//{
//    setMouseRegion(QRegion(rect));
//}

void ViewHelper::setDefaultRegion()
{
    setMouseRegion(QRegion( (540-480)/2, (960-240)/2 ,480, 240));
}

void ViewHelper::hideWindow()
{
    view->hide();
}
