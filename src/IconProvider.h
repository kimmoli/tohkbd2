#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H

#include <sailfishapp.h>
#include <QQuickImageProvider>
#include <QPainter>
#include <QColor>

class IconProvider : public QQuickImageProvider
{
public:
    IconProvider() : QQuickImageProvider(QQuickImageProvider::Pixmap)
    {
    }

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
    {
        QStringList parts = id.split('?');

        QPixmap sourcePixmap(SailfishApp::pathTo("qml/icons/" + parts.at(0) + ".png").toString(QUrl::RemoveScheme));

        if (size)
            *size  = sourcePixmap.size();

        if (parts.length() > 1)
            if (QColor::isValidColor(parts.at(1)))
            {
                QPainter painter(&sourcePixmap);
                painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
                painter.fillRect(sourcePixmap.rect(), parts.at(1));
                painter.end();
            }

        if (requestedSize.width() > 0 && requestedSize.height() > 0)
            return sourcePixmap.scaled(requestedSize.width(), requestedSize.height(), Qt::IgnoreAspectRatio);
        else
            return sourcePixmap;
    }
};

#endif // ICONPROVIDER_H
