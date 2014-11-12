#include "readerwriter.h"

ReaderWriter::ReaderWriter(QObject *parent) :
    QObject(parent)
{
}


void ReaderWriter::write(const QDBusMessage &msg)
{
    QList<QVariant> args = msg.arguments();

    if (args.count() == 2)
    {
        printf("write %s = %s\n", qPrintable(args.at(0).toString()), qPrintable(args.at(1).toString()));

        MGConfItem ci(args.at(0).toString());

        ci.set(args.at(1));
    }
}

QString ReaderWriter::read(const QDBusMessage &msg)
{
    QList<QVariant> args = msg.arguments();

    if (args.count() == 1)
    {
        MGConfItem ci(args.at(0).toString());

        printf("read %s = %s\n", qPrintable(args.at(0).toString()), qPrintable(ci.value().toString()));

        return ci.value().toString();
    }
    else
        return QString();
}

