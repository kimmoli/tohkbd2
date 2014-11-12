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
#include "readerwriter.h"

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

    printf("Starting dconf reader writer daemon version %s\n", APPVERSION);

    printf("Environment %s\n", qPrintable(getenv ("DBUS_SESSION_BUS_ADDRESS")));

    if (!QDBusConnection::sessionBus().isConnected())
    {
        printf("Cannot connect to the D-Bus sessionBus\n%s\n", qPrintable(QDBusConnection::sessionBus().lastError().message()));
        exit(EXIT_FAILURE);
    }
    printf("Connected to D-Bus sessionbus\n");

    if (!QDBusConnection::systemBus().registerService(SERVICE_NAME))
    {
        printf("Cannot register service to systemBus\n%s\n", qPrintable(QDBusConnection::systemBus().lastError().message()));
        exit(EXIT_FAILURE);
    }

    printf("Registered %s to D-Bus systembus\n", SERVICE_NAME);

    ReaderWriter rw;

    QDBusConnection::systemBus().registerObject("/", &rw, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);

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
            /* rehash the server */
            printf("Received signal SIGHUP\n");
            break;
        case SIGTERM:
            /* finalize the server */
            printf("Received signal SIGTERM\n");
            controlVdd(0);
            exit(0);
            break;
    }
}
