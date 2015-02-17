#ifndef VIEWHELPER_H
#define VIEWHELPER_H

#include <QObject>
#include <QQuickView>
#include <QProcess>
#include <QThread>
#include <mlite5/MDesktopEntry>
#include <QVariantMap>
#include <QVariantList>

class ViewHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int currentApp READ getCurrentApp NOTIFY currentAppChanged())

public:
    Q_INVOKABLE QVariantList getCurrentShortcuts();
    explicit ViewHelper(QQuickView *parent = 0);
    void setDefaultRegion();


signals:
    void currentAppChanged();

public slots:
//    void setTouchRegion(const QRect &rect);
    void detachWindow();
    void hideWindow();
    void nextApp();
    void launchApplication(int n);

private:
    void setMouseRegion(const QRegion &region);
    QQuickView *view;

    int getCurrentApp();
    int m_numberOfApps;
    int m_currentApp;
    QStringList apps;

};

#endif // VIEWHELPER_H
