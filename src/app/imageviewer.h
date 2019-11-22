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
    void setLegend(const QLinearGradient& gradient, const QString& lmin,const QString& lmax);
protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
    virtual void drawForeground(QPainter * painter,const QRectF& rect);
    QGraphicsScene scene_;
    QGraphicsPixmapItem* pixmap_item_;
    QList<QGraphicsItem*> markers_;
    int scalefactor_;
    QLinearGradient legend_gradient_;
    QString legend_min_;
    QString legend_max_;
};

#endif // IMAGEVIEWER_H
