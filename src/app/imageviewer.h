#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QGraphicsView>
#include <QGraphicsScene>

class ImageViewer : public QGraphicsView
{
public:
    ImageViewer(QWidget *parent = nullptr);
public slots:
    void load(const QString& path);
    void addMarker(const QPointF& pos, qreal r, const QString& tooltip, const QColor & color);
    void clear();
    void clearMarkers();
protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
    QGraphicsScene scene_;
    QGraphicsPixmapItem* pixmap_item_;
    QList<QGraphicsItem*> markers_;
    int scalefactor_;
};

#endif // IMAGEVIEWER_H
