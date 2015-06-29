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
    Q_PROPERTY(QString remorseText READ getRemorseText NOTIFY remorseTextChanged)

public:
    explicit ViewHelper(QQuickView *parent = 0);
    void setDefaultRegion();
    Q_INVOKABLE QVariantList getCurrentApps();
    Q_INVOKABLE void launchApplication(int n);
    Q_INVOKABLE void setCurrentApp(int n);
    Q_INVOKABLE void setNumberOfApps(int n);
    Q_INVOKABLE void setTouchRegion(const QRect &rect);
    Q_INVOKABLE void remorseTriggered();
    Q_INVOKABLE void remorseCancelled();

signals:
    void currentAppChanged();
    void numberOfAppsChanged();
    void visibleChanged();
    void _launchApplication(const QString &desktopFilename);
    void startRemorse();
    void remorseTextChanged();

public slots:
    void detachWindow();
    void hideWindow();
    void showWindow();
    void nextApp();
    void requestActionWithRemorse(const QString &action);

private:
    void setMouseRegion(const QRegion &region);
    QQuickView *view;

    int getCurrentApp();
    int getNumberOfApps();
    bool getVisible();
    QString getRemorseText() { return m_remorseText; }

    int m_numberOfApps;
    int m_currentApp;
    bool m_visible;

    QVariantList apps;
    QStringList appsDesktopFiles;

    QStringList mruList;

    QString m_remorseAction;
    QString m_remorseText;
};

#endif // VIEWHELPER_H
