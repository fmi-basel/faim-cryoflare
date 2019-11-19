#include <QDebug>
#include <QFileInfo>
#include <QWheelEvent>
#include <QGraphicsPixmapItem>
#include "imageviewer.h"

ImageViewer::ImageViewer(QWidget *parent):
    QGraphicsView(parent),
    scene_(),
    pixmap_item_(),
    markers_(),
    scalefactor_(1)
{
    pixmap_item_=scene_.addPixmap(QPixmap());
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragMode(ScrollHandDrag);
}

void ImageViewer::load(const QString &path)
{
    clear();
    setScene(&scene_);
    if(QFileInfo::exists(path)){
        pixmap_item_->setPixmap(QPixmap(path));
        scene_.setSceneRect(pixmap_item_->boundingRect());
         fitInView(scene_.sceneRect(),Qt::KeepAspectRatio);
    }
}

void ImageViewer::addMarker(const QPointF &pos, qreal r, const QString &tooltip, const QColor &color)
{
    QColor color_no_alpha=color;
    color_no_alpha.setAlpha(255);
    QGraphicsItem * marker=scene_.addEllipse(QRectF(pos-QPointF(r,r),pos+QPointF(r,r)),QPen(color_no_alpha),QBrush(color ));
    marker->setToolTip(tooltip);
    markers_.append(marker);
}

void ImageViewer::clear()
{
    pixmap_item_->setPixmap(QPixmap());
    clearMarkers();
}

void ImageViewer::clearMarkers()
{
    foreach(QGraphicsItem* item,markers_){
        scene_.removeItem(item);
    }
    markers_.clear();
}

void ImageViewer::resizeEvent(QResizeEvent *event)
{
    fitInView(scene_.sceneRect(),Qt::KeepAspectRatio);
    QGraphicsView::resizeEvent(event);
}

void ImageViewer::wheelEvent(QWheelEvent *event)
{
    if(event->delta()>0){
        ++scalefactor_;
        scale(2,2);
    }else{
        --scalefactor_;
        scale(0.5,0.5);
    }
}

