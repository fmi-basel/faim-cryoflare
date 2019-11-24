//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2017-2019 by the CryoFLARE Authors
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

#include "chartview.h"
#include "imagetablemodel.h"

#include <QPushButton>
#include <QTimer>
#include <QValueAxis>

ChartView::ChartView(QWidget *parent):
    QtCharts::QChartView(parent),
    model_(),
    active_column_(0),
    fit_button_(new QPushButton("")),
    color_(255,127,127),
    selected_color_(23,159,223),
    selecting_(false),
    rubberband_(new QRubberBand(QRubberBand::Rectangle,this)),
    timer_(new QTimer(this))
{
    QIcon icon;
    icon.addFile(":/icons/fit-to-window.png",QSize(),QIcon::Normal,QIcon::On);
    icon.addFile(":/icons/fit-to-window-inactive.png",QSize(),QIcon::Normal,QIcon::Off);
    fit_button_->setCheckable(true);
    fit_button_->setChecked(true);
    fit_button_->setIcon(icon);
    connect(fit_button_,&QPushButton::clicked,chart(),&QtCharts::QChart::zoomReset);
    setRubberBand(QChartView::RectangleRubberBand);
    scene()->addWidget(fit_button_);
    timer_->setSingleShot(true);
    timer_->setInterval(0);
    connect(timer_,&QTimer::timeout,this,&ChartView::drawChart_);
    chart()->legend()->hide();
}

void ChartView::setModel(ImageTableModel *model)
{
    if(model_){
        disconnect(model_,&ImageTableModel::dataChanged,this,&ChartView::update);
    }
    model_=model;
    connect(model_,&ImageTableModel::dataChanged,this,&ChartView::update);
}

void ChartView::setActiveColumn(int column)
{
    active_column_=column;
    fit_button_->setChecked(true);
    update();
}

void ChartView::enableSelection(bool selecting)
{
    selecting_=selecting;
}

void ChartView::update()
{
    timer_->start();
}

void ChartView::drawChart_()
{
    if(!model_){
        return;
    }
    bool keep_zoom=!fit_button_->isChecked() && ! chart()->series().empty();
    qreal xmin=0,xmax=0,ymin=0,ymax=0;
    if(keep_zoom){
        QtCharts::QValueAxis *axisx=dynamic_cast<QtCharts::QValueAxis*>(chart()->axes(Qt::Horizontal)[0]);
        QtCharts::QValueAxis *axisy=dynamic_cast<QtCharts::QValueAxis*>(chart()->axes(Qt::Vertical)[0]);
        xmin=axisx->min();
        ymin=axisy->min();
        xmax=axisx->max();
        ymax=axisy->max();
    }
    chart()->removeAllSeries();
    drawSeries_();
    chart()->createDefaultAxes();
    if(keep_zoom){
        //need to zoom in first, otherwise resetting zoom won't do anything afterwards
        chart()->zoomIn();
        QtCharts::QValueAxis *axisx=dynamic_cast<QtCharts::QValueAxis*>(chart()->axes(Qt::Horizontal)[0]);
        QtCharts::QValueAxis *axisy=dynamic_cast<QtCharts::QValueAxis*>(chart()->axes(Qt::Vertical)[0]);
        axisx->setRange(xmin,xmax);
        axisy->setRange(ymin,ymax);
    }
    chart()->setTitle(model_->headerData(active_column_,Qt::Horizontal,Qt::DisplayRole).toString());
}

void ChartView::drawSeries_()
{

}

void ChartView::mousePressEvent(QMouseEvent *event)
{
    if(event->button()==Qt::RightButton){
        selecting_=true;
        QRect plot_area =chart()->plotArea().toRect();
        rubberband_start_=event->pos();
        rubberband_start_.setY(plot_area.top());
        rubberband_->setGeometry(QRect(rubberband_start_,QSize()));
        rubberband_->show();
    }else{
        QChartView::mousePressEvent(event);
    }
}

void ChartView::mouseMoveEvent(QMouseEvent *event)
{
    if(selecting_){
        QRect plot_area =chart()->plotArea().toRect();
        int width = event->pos().x()-rubberband_start_.x();
        rubberband_->setGeometry(QRect(rubberband_start_,QSize(width,plot_area.height())).normalized());
    }else{
        QChartView::mouseMoveEvent(event);
    }
}


void ChartView::mouseReleaseEvent(QMouseEvent *event)
{
    if(selecting_){
        selecting_=false;
        rubberband_->hide();
        QRect selection=rubberband_->geometry();
        selecting_=false;
        deselectData_(mapToChartValue_(selection.topLeft()).x(),mapToChartValue_(selection.bottomRight()).x(),event->modifiers() & Qt::ShiftModifier);
    }else{
        QChartView::mouseReleaseEvent(event);
        if(chart()->isZoomed()){
            fit_button_->setChecked(false);
        }
    }
}

QPointF ChartView::mapToChartValue_(const QPoint &p) const
{
    return chart()->mapToValue(chart()->mapFromScene(mapToScene(p)));
}

QColor ChartView::selectedColor() const
{
    return selected_color_;
}

void ChartView::setSelectedColor(const QColor &selected_color)
{
    selected_color_ = selected_color;
}

QColor ChartView::color() const
{
    return color_;
}

void ChartView::setColor(const QColor &color)
{
    color_ = color;
}
