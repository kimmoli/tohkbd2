/*
 * (C) 2014 Kimmo Lindholm <kimmo.lindholm@gmail.com> Kimmoli
 *
 * Main, Daemon functions
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "tohkeyboard.h"
#include "toh.h"
#include "adaptor.h"

#include <QtCore/QCoreApplication>
#include <QDBusConnection>

static void daemonize();
static void signalHandler(int sig);

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    daemonize();

    setlinebuf(stdout);
    setlinebuf(stderr);

    printf("Starting tohkbd daemon version %s\n", APPVERSION);

    QTranslator translator;
    translator.load("translations_" + QLocale::system().name(), "/usr/share/harbour-tohkbd2/i18n");
    app.installTranslator(&translator);

    /* Check that we can connect to dbus systemBus and sessionBus */

    QDBusConnection dbusSystemBus = QDBusConnection::systemBus();
    if (!dbusSystemBus.isConnected())
    {
        printf("Cannot connect to the D-Bus systemBus\n%s\n",
               qPrintable(dbusSystemBus.lastError().message()));
        sleep(3);
        exit(EXIT_FAILURE);
    }
    printf("Connected to D-Bus systembus\n");

    QDBusConnection dbusSessionBus = QDBusConnection::sessionBus();
    if (!dbusSessionBus.isConnected())
    {
        printf("Cannot connect to the D-Bus sessionBus\n%s\n",
               qPrintable(dbusSessionBus.lastError().message()));
        sleep(3);
        exit(EXIT_FAILURE);
    }

    printf("Connected to D-Bus sessionbus\n");


    Tohkbd tohkbd;

    /* Register to dbus systemBus */
    new Tohkbd2Adaptor(&tohkbd);
    tohkbd.registerDBus();

    /* Nokia MCE display_status_ind
     * used to enable and disable keyboard when display is on or off
     */
    dbusSystemBus.connect("com.nokia.mce", "/com/nokia/mce/signal", "com.nokia.mce.signal", "display_status_ind",
                          &tohkbd, SLOT(handleDisplayStatus(const QDBusMessage&)));

    return app.exec();
}

static void daemonize()
{
    /* Change the file mode mask */
    umask(0);

    /* Change the current working directory */
    if ((chdir("/tmp")) < 0)
        exit(EXIT_FAILURE);

    /* register signals to monitor / ignore */
    signal(SIGCHLD,SIG_IGN); /* ignore child */
    signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);
    signal(SIGHUP,signalHandler); /* catch hangup signal */
    signal(SIGTERM,signalHandler); /* catch kill signal */
}


static void signalHandler(int sig) /* signal handler function */
{
    switch(sig)
    {
        case SIGHUP:
            printf("Received signal SIGHUP\n");
            break;

        case SIGTERM:
            printf("Received signal SIGTERM\n");
            controlVdd(0);
            QCoreApplication::quit();
            break;
    }
}
