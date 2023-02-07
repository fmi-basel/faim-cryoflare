//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2020 by the CryoFLARE Authors
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
#include "imagetablemodel.h"
#include "linearchartview.h"

#include <QtMath>
#include <QScatterSeries>
#include <QToolTip>
#include <QElapsedTimer>

LinearChartView::LinearChartView(QWidget *parent):
    ChartView (parent)
{

}

void LinearChartView::drawSeries_()
{
    QList<QPointF> selected_data;
    QList<QPointF> unselected_data;
    for(int i=0;i<model_->rowCount();++i){
        QVariant val=model_->data(model_->index(i,active_column_),ImageTableModel::SortRole);
        if(val.canConvert<float>() && val.toString()!=QString("")){
            qreal fval=val.toDouble();
            QPointF p(i,fval);
            bool export_flag=model_->data(model_->index(i,0),ImageTableModel::SortRole)==1;
            if(export_flag){
                selected_data.append(p);
            } else {
                unselected_data.append(p);
            }
        }
    }

    QtCharts::QScatterSeries *unselected_series = new QtCharts::QScatterSeries();
    unselected_series->setColor(color_);
    unselected_series->setPen(QPen(color_,0));
    unselected_series->setMarkerSize(7);
    unselected_series->append(unselected_data);
    chart()->addSeries(unselected_series);
    connect(unselected_series,&QtCharts::QScatterSeries::hovered,this,&LinearChartView::showDetails);
    connect(unselected_series,&QtCharts::QScatterSeries::doubleClicked,this,&LinearChartView::handleDoubleClicked);

    QtCharts::QScatterSeries *selected_series = new QtCharts::QScatterSeries();
    selected_series->setColor(selected_color_);
    selected_series->setPen(QPen(selected_color_,0));
    selected_series->setMarkerSize(7);
    selected_series->append(selected_data);
    chart()->addSeries(selected_series);
    connect(selected_series,&QtCharts::QScatterSeries::hovered,this,&LinearChartView::showDetails);
    connect(selected_series,&QtCharts::QScatterSeries::doubleClicked,this,&LinearChartView::handleDoubleClicked);

}

void LinearChartView::deselectData_(float start, float end, bool invert)
{
    int istart=std::max(0,static_cast<int>(start));
    int iend=std::min(model_->rowCount()-1,static_cast<int>(end));
    if(invert){
        for(int i=istart;i<=iend;++i){
            model_->setData(model_->index(i,0),Qt::Unchecked,Qt::CheckStateRole);
        }
    }else{
        for(int i=0;i<istart;++i){
            model_->setData(model_->index(i,0),Qt::Unchecked,Qt::CheckStateRole);
        }
        for(int i=iend+1;i<model_->rowCount();++i){
            model_->setData(model_->index(i,0),Qt::Unchecked,Qt::CheckStateRole);
        }
    }
}

void LinearChartView::showDetails(const QPointF &pf, bool state)
{
    if(!state ){
        return;
    }
    QVariant mic_name=model_->data(model_->index(qFloor(pf.x()),1),ImageTableModel::SortRole);
    if(mic_name.canConvert<QString>() && mic_name.toString()!=QString("") ){
        QRect tooltip_rect(chart()->mapToPosition(pf).toPoint(),QSize(5,5));
        QToolTip::showText(mapToGlobal(chart()->mapToPosition(pf).toPoint()),QString("%1: %2").arg(mic_name.toString()).arg(pf.y()),this,tooltip_rect,10000);
    }  
}

void LinearChartView::handleDoubleClicked(const QPointF &pf)
{
    emit indexClicked(pf.toPoint().x());
}

