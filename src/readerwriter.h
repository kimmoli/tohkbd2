#ifndef READERWRITER_H
#define READERWRITER_H

#include <QObject>
#include <QtDBus/QtDBus>
#include <mlite5/MGConfItem>

#define SERVICE_NAME "com.kimmoli.dconfreaderwriter"

class ReaderWriter : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", SERVICE_NAME)

public:
    explicit ReaderWriter(QObject *parent = 0);

signals:

public slots:
    void write(const QDBusMessage &msg);
    QString read(const QDBusMessage &msg);

};

#endif // READERWRITER_H
