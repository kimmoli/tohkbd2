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
    Q_PROPERTY(int numberOfApps READ getNumberOfApps NOTIFY numberOfAppsChanged())
    Q_PROPERTY(bool visible READ getVisible NOTIFY visibleChanged)

public:
    explicit ViewHelper(QQuickView *parent = 0);
    void setDefaultRegion();
    Q_INVOKABLE QVariantList getCurrentApps();
    Q_INVOKABLE void launchApplication(int n);
    Q_INVOKABLE void setCurrentApp(int n);
    Q_INVOKABLE void setNumberOfApps(int n);
    Q_INVOKABLE void setTouchRegion(const QRect &rect);

signals:
    void currentAppChanged();
    void numberOfAppsChanged();
    void visibleChanged();
    void _launchApplication(const QString &desktopFilename);

public slots:
    void detachWindow();
    void hideWindow();
    void showWindow();
    void nextApp();

private:
    void setMouseRegion(const QRegion &region);
    QQuickView *view;

    int getCurrentApp();
    int getNumberOfApps();
    bool getVisible();

    int m_numberOfApps;
    int m_currentApp;
    bool m_visible;

    QVariantList apps;
    QStringList appsDesktopFiles;

    QStringList mruList;

};

#endif // VIEWHELPER_H
