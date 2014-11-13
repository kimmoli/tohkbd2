/*
 * (C) 2014 Kimmo Lindholm <kimmo.lindholm@gmail.com> Kimmoli
 *
 * Main, Daemon functions
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "readerwriter.h"
#include "adaptor.h"
#include <QtCore/QCoreApplication>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    setlinebuf(stdout);
    setlinebuf(stderr);

    printf("Starting tohkbd2user daemon version %s\n", APPVERSION);

    ReaderWriter rw;
    new Tohkbd2userAdaptor(&rw);

    rw.registerDBus();

    return app.exec();
}
