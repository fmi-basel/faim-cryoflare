//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFlare
//
// Copyright (C) 2017-2018 by the CryoFlare Authors
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
// along with CryoFlare.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------------

#include <QtDebug>
#include <QMouseEvent>
#include <QGraphicsItem>
#include "positionchartview.h"
#include <algorithm>

PositionChartView::PositionChartView(QWidget *parent):
    QGraphicsView(parent)
{
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);
    QLinearGradient linear_gradient(QPointF(0, 0), QPointF(0, 1));
    linear_gradient.setColorAt(0, QColor(5,97,137));
    linear_gradient.setColorAt(1, QColor(16,27,50));
    linear_gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    setBackgroundBrush(QBrush(linear_gradient));
}

void PositionChartView::setScene(QGraphicsScene *s)
{
    QGraphicsView::setScene(s);
    fitInView(scene()->sceneRect(),Qt::KeepAspectRatio );
}

void PositionChartView::resizeEvent(QResizeEvent *event)
{
    if(! scene()){
        return;
    }
    fitInView(scene()->sceneRect(),Qt::KeepAspectRatio );
}

void PositionChartView::mousePressEvent(QMouseEvent *event)
{
    if (QGraphicsItem *item = itemAt(event->pos())) {
        if(item->toolTip()!=""){
            emit clicked(item->toolTip().toInt());
        }
     }
}
