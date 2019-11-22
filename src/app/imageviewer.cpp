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
    scalefactor_(1),
    legend_gradient_(),
    legend_min_(),
    legend_max_()
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

void ImageViewer::setLegend(const QLinearGradient &gradient, const QString &lmin, const QString &lmax)
{
    legend_gradient_=gradient;
    legend_gradient_.setStart(QPointF(40,15));
    legend_gradient_.setFinalStop(QPointF(240,15));
    legend_min_=lmin;
    legend_max_=lmax;
    scene_.update();

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

void ImageViewer::drawForeground(QPainter *painter, const QRectF &rect)
{
    if(! markers_.empty()){
        painter->setPen(Qt::black);
        painter->resetTransform();
        painter->setBrush(legend_gradient_);
        painter->setFont(QFont("Arial", 12));
        painter->drawRect(QRectF(QPointF(40,10),QPointF(240,20)));
        painter->drawText(QRectF(QPointF(0,25),QPointF(80,40)), Qt::AlignCenter, legend_min_);
        painter->drawText(QRectF(QPointF(200,25),QPointF(280,40)), Qt::AlignCenter, legend_max_);
    }
}

