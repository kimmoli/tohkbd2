#ifndef READERWRITER_H
#define READERWRITER_H

#include <QObject>
#include <QtDBus/QtDBus>
#include <mlite5/MGConfItem>

#define SERVICE_NAME "com.kimmoli.tohkbd2user"

class QDBusInterface;
class ReaderWriter : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", SERVICE_NAME)

public:
    explicit ReaderWriter(QObject *parent = 0);
    virtual ~ReaderWriter();
    void registerDBus();

signals:

public slots:
    QString getActiveLayout();
    void setActiveLayout(const QString &value);
    void quit();

private:
    bool m_dbusRegistered;

};

#endif // READERWRITER_H
