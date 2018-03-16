#include <algorithm>
#include <QGraphicsEllipseItem>
#include "positionchart.h"
#include <QtDebug>

PositionChart::PositionChart( QObject *parent):
    QGraphicsScene(parent),
    items_(),
    gradient_stops_(),
    minval_(0),
    valrange_(1),
    min_label_(),
    max_label_()
{
    gradient_stops_ << QGradientStop(0.0,QColor::fromRgbF(1, 0, 0, 1)) << QGradientStop(0.3333,QColor::fromRgbF(1, 1, 0, 1)) << QGradientStop(0.6666,QColor::fromRgbF(0, 1, 0, 1)) << QGradientStop(1.0,QColor::fromRgbF(0, 0, 1, 1));
}

void PositionChart::addPositions(const QPainterPath & path, const QList<QPointF>& pos)
{
    int i=1;
    foreach(QPointF p, pos){
        QGraphicsPathItem* item=dynamic_cast<QGraphicsPathItem*>(addPath(path));
        item->setPos(p);
        item->setToolTip(QString("%1").arg(i));
        item->setZValue(i);
        items_.append(item);
        ++i;
    }
    QRectF rect=itemsBoundingRect();
    QPointF legend_tl=rect.topRight()+QPointF(0.1*rect.height(),0.1*rect.height());
    QPointF legend_br=rect.bottomRight()+QPointF(0.2*rect.height(),-0.1*rect.height());
    QLinearGradient gradient(legend_br-QPointF(0.05*rect.height(),0),legend_tl+QPointF(0.05*rect.height(),0));
    gradient.setStops(gradient_stops_);
    addRect(QRectF(legend_tl,legend_br),QPen(),QBrush(gradient));
    min_label_=addText(QString("%1").arg(minval_));
    min_label_->setPos(legend_br+QPointF(0.05*rect.height(),-0.05*rect.height()));
    max_label_=addText(QString("%1").arg(minval_+valrange_));
    max_label_->setPos(legend_tl+QPointF(0.15*rect.height(),-0.05*rect.height()));
    setSceneRect(itemsBoundingRect());
}

void PositionChart::setMinMaxValue(float minval, float maxval)
{
    minval_=minval;
    min_label_->setPlainText(QString("%1").arg(minval));
    max_label_->setPlainText(QString("%1").arg(maxval));
    valrange_=maxval-minval_;
    setSceneRect(itemsBoundingRect());
}

void PositionChart::setValues(const QVector<float>& values)
{
    float minval=std::numeric_limits<float>::max();
    float maxval=std::numeric_limits<float>::lowest();
    Q_FOREACH(float val, values){
        if(! qIsNaN(val)){
            minval=minval>val?val:minval;
            maxval=maxval<val?val:maxval;
        }
    }
    setMinMaxValue(minval,maxval);
    int imax=std::min(values.size(),items_.size());
    Q_FOREACH(QGraphicsPathItem* item,items_){
        item->setBrush(QColor(255,255,255));
    }

    for(int i=0;i<imax;++i){
       items_[i]->setBrush(QBrush(colorAt(values[i])));
    }
}

QColor PositionChart::colorAt(float value)
{
    if(qIsNaN(value)){
        return QColor::fromRgbF(1.0,1.0,1.0,1.0);
    }
    float relval=std::max(0.0f,std::min(1.0f,(value-minval_)/valrange_));
    if(relval==1.0f){
        return gradient_stops_.last().second;
    }
    float interval=1.0/(gradient_stops_.size()-1);
    int index=static_cast<int>(relval/interval);
    float rel_offset=relval-index*interval;
    float irel_offset=1.0-rel_offset;
    QColor start=gradient_stops_[index].second;
    QColor end=gradient_stops_[index+1].second;
    qreal red=std::max(0.0,std::min(1.0,start.redF()*irel_offset+end.redF()*rel_offset));
    qreal green=std::max(0.0,std::min(1.0,start.greenF()*irel_offset+end.greenF()*rel_offset));
    qreal blue=std::max(0.0,std::min(1.0,start.blueF()*irel_offset+end.blueF()*rel_offset));
    qreal alpha=std::max(0.0,std::min(1.0,start.alphaF()*irel_offset+end.alphaF()*rel_offset));
    return QColor::fromRgbF(red,green,blue,alpha);
}
