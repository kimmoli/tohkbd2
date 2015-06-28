#include <stdio.h>
#include "applauncher.h"
#include <mlite5/MDesktopEntry>
#include <QSharedPointer>

#include <QThread>
#include <QProcess>

AppLauncher::AppLauncher(QObject *parent) :
    QObject(parent)
{
}

void AppLauncher::launchApplication(const QString &desktopFilename)
{
    QSharedPointer<MDesktopEntry> app;

    app = QSharedPointer<MDesktopEntry>(new MDesktopEntry(desktopFilename));

    if (!app->isValid())
    {
        printf("tohkbd2-user: AppLauncher: invalid application: %s\n", qPrintable(desktopFilename));
        emit launchFailed();
        return;
    }

    printf("tohkbd2-user: AppLauncher: starting: %s\n" ,qPrintable(app->name()));

    emit launchSuccess(app->name());

    QProcess proc;
    proc.startDetached("/usr/bin/xdg-open" , QStringList() << desktopFilename);

    QThread::msleep(100);
}

