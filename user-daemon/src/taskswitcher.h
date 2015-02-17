#ifndef TASKSWITCHER_H
#define TASKSWITCHER_H

#include <QObject>

#include <QProcess>
#include <QThread>
#include <mlite5/MDesktopEntry>
#include <QVariantMap>
#include <QVariantList>

class Taskswitcher : public QObject
{
    Q_OBJECT
public:
    explicit Taskswitcher(QObject *parent = 0);
    Q_INVOKABLE QVariantList getCurrentShortcuts();

public slots:
    void launchApplication(const QString &app);
};

#endif // TASKSWITCHER_H
