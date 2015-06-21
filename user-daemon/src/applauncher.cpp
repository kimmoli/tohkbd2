#include <stdio.h>
#include "applauncher.h"
#include <mlite5/MDesktopEntry>
#include <contentaction5/contentaction.h>

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

    ContentAction::Action action;
    action = ContentAction::Action::launcherAction(app, QStringList());
    action.trigger();
}

