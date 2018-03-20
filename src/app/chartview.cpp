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

#include "chartview.h"

ChartView::ChartView(QWidget *parent):
    QtCharts::QChartView(parent),
    selecting_(false),
    rubberband_(new QRubberBand(QRubberBand::Rectangle,this))
{

}

void ChartView::enableSelection(bool selecting)
{
    selecting_=selecting;
}

void ChartView::mousePressEvent(QMouseEvent *event)
{
    if(selecting_){
        QRect plot_area =chart()->plotArea().toRect();
        rubberband_start_=event->pos();
        rubberband_start_.setY(plot_area.top());
        rubberband_->setGeometry(QRect(rubberband_start_,QSize()));
        rubberband_->show();
    }
}

void ChartView::mouseMoveEvent(QMouseEvent *event)
{
    if(selecting_){
        QRect plot_area =chart()->plotArea().toRect();
        int width = event->pos().x()-rubberband_start_.x();
        rubberband_->setGeometry(QRect(rubberband_start_,QSize(width,plot_area.height())).normalized());
    }
}


void ChartView::mouseReleaseEvent(QMouseEvent *event)
{
    if(selecting_){
        rubberband_->hide();
        QRect selection=rubberband_->geometry();
        selecting_=false;
        emit selected(mapToChartValue_(selection.topLeft()).x(),mapToChartValue_(selection.bottomRight()).x(),event->modifiers() & Qt::ShiftModifier);
    }
}

QPointF ChartView::mapToChartValue_(const QPoint &p) const
{
    return chart()->mapToValue(chart()->mapFromScene(mapToScene(p)));
}
