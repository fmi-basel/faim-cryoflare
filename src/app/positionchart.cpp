//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2017-2018 by the CryoFLARE Authors
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3.0 of the License.
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License
// along with CryoFLARE.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------------

#include <algorithm>
#include <QGraphicsEllipseItem>
#include "positionchart.h"
#include <QtDebug>

PositionChart::PositionChart( QObject *parent):
    QGraphicsScene(parent),
    min_label(addSimpleText("")),
    max_label(addSimpleText("")),
    items_(),
    gradient_stops_(),
    minval_(0),
    valrange_(1)
{
    gradient_stops_ << QGradientStop(0.0,QColor::fromRgbF(1, 0, 0, 1)) << QGradientStop(0.3333,QColor::fromRgbF(1, 1, 0, 1)) << QGradientStop(0.6666,QColor::fromRgbF(0, 1, 0, 1)) << QGradientStop(1.0,QColor::fromRgbF(0, 0, 1, 1));
}

void PositionChart::addPositions(const QPainterPath & path, const QHash<int, QPointF> &pos, bool back)
{
    for(QHash<int,QPointF>::const_iterator i = pos.constBegin();i != pos.constEnd();++i) {
        QGraphicsPathItem* item=dynamic_cast<QGraphicsPathItem*>(addPath(path));
        item->setPos(i.value());
        item->setToolTip(QString("%1").arg(i.key()));
        item->setZValue(i.key());
        item->setFlag(QGraphicsItem::ItemIsSelectable,true);
        items_[i.key()]=item;
    }
    QRectF rect=itemsBoundingRect();
    QPointF legend_tl=rect.topRight()+QPointF(0.1*rect.height(),0.2*rect.height());
    QPointF legend_br=rect.bottomRight()+QPointF(0.2*rect.height(),-0.1*rect.height());
    QLinearGradient gradient(legend_br-QPointF(0.05*rect.height(),0),legend_tl+QPointF(0.05*rect.height(),0));
    gradient.setStops(gradient_stops_);
    addRect(QRectF(legend_tl,legend_br),QPen(),QBrush(gradient));
    if(back){

        QGraphicsPixmapItem* back=addPixmap(QPixmap(":/icons/draw-arrow-back.png"));
        back->setToolTip("back");
        back->setPos(legend_tl+QPointF(0,-0.2*rect.height()));
        back->setFlag(QGraphicsItem::ItemIgnoresTransformations,true);
        back->setFlag(QGraphicsItem::ItemIsSelectable,true);
    }
    min_label=addSimpleText(QString("%1").arg(minval_));
    min_label->setPos(legend_br+QPointF(0.05*rect.height(),-0.05*rect.height()));
    max_label=addSimpleText(QString("%1").arg(minval_+valrange_));
    max_label->setPos(legend_tl+QPointF(0.15*rect.height(),-0.05*rect.height()));
    min_label->setBrush(QColor("#e6e6e6"));
    max_label->setBrush(QColor("#e6e6e6"));
    min_label->setFlag(QGraphicsItem::ItemIgnoresTransformations,true);
    max_label->setFlag(QGraphicsItem::ItemIgnoresTransformations,true);
    setSceneRect(itemsBoundingRect());
}

void PositionChart::setMinMaxValue(double minval, double maxval)
{
    minval_=minval;
    min_label->setText(QString("%1").arg(minval));
    max_label->setText(QString("%1").arg(maxval));
    valrange_=maxval-minval_;
    setSceneRect(itemsBoundingRect());
}

void PositionChart::setValues(const QHash<int, double> &values)
{
    double minval=std::numeric_limits<double>::max();
    double maxval=std::numeric_limits<double>::lowest();
    foreach(double val, values){
        minval=minval>val?val:minval;
        maxval=maxval<val?val:maxval;
    }
    setMinMaxValue(minval,maxval);
    foreach(QGraphicsPathItem* item,items_){
        item->setBrush(QBrush());
    }
    for(QHash<int,double>::const_iterator i = values.constBegin();i != values.constEnd();++i) {
        if(items_.contains(i.key())){
            items_[i.key()]->setBrush(QBrush(colorAt(i.value())));
        }
    }
}

void PositionChart::clear()
{
    QGraphicsScene::clear();
    items_.clear();
}

QColor PositionChart::colorAt(double value)
{
    if(qIsNaN(value)){
        return QColor::fromRgbF(1.0,1.0,1.0,1.0);
    }
    double relval=std::max(0.0,std::min(1.0,(value-minval_)/valrange_));
    if(relval==1.0){
        return gradient_stops_.last().second;
    }
    double interval=1.0/(gradient_stops_.size()-1);
    int index=static_cast<int>(relval/interval);
    double rel_offset=relval-index*interval;
    double irel_offset=1.0-rel_offset;
    QColor start=gradient_stops_[index].second;
    QColor end=gradient_stops_[index+1].second;
    qreal red=std::max(0.0,std::min(1.0,start.redF()*irel_offset+end.redF()*rel_offset));
    qreal green=std::max(0.0,std::min(1.0,start.greenF()*irel_offset+end.greenF()*rel_offset));
    qreal blue=std::max(0.0,std::min(1.0,start.blueF()*irel_offset+end.blueF()*rel_offset));
    qreal alpha=std::max(0.0,std::min(1.0,start.alphaF()*irel_offset+end.alphaF()*rel_offset));
    return QColor::fromRgbF(red,green,blue,alpha);
}
