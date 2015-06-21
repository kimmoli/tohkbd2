#ifndef APPLAUNCHER_H
#define APPLAUNCHER_H

#include <QObject>

class AppLauncher : public QObject
{
    Q_OBJECT
public:
    explicit AppLauncher(QObject *parent = 0);


public slots:
    void launchApplication(const QString &desktopFilename);

signals:
    void launchSuccess(const QString &appName);
    void launchFailed();
};

#endif // APPLAUNCHER_H
