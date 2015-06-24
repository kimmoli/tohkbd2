#include <stdio.h>
#include "applauncher.h"
#include <mlite5/MDesktopEntry>
#include <QSharedPointer>

#ifdef USECONTENTACTION
  #include <contentaction5/contentaction.h>
#else
  #include <QThread>
  #include <QProcess>
#endif

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

#ifdef USECONTENTACTION
    ContentAction::Action action;
    action = ContentAction::Action::launcherAction(app, QStringList());
    action.trigger();
#else
    QProcess proc;
    proc.startDetached("/usr/bin/xdg-open" , QStringList() << desktopFilename);

    QThread::msleep(100);
#endif
}

