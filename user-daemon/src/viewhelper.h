#ifndef VIEWHELPER_H
#define VIEWHELPER_H

#include <QObject>
#include <QQuickView>

class ViewHelper : public QObject
{
    Q_OBJECT
public:
    explicit ViewHelper(QQuickView *parent = 0);
    void setDefaultRegion();

public slots:
//    void setTouchRegion(const QRect &rect);
    void detachWindow();
    void hideWindow();

private:
    void setMouseRegion(const QRegion &region);
    QQuickView *view;

};

#endif // VIEWHELPER_H
