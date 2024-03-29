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
#include "histogram.h"
#include "histogramchartview.h"
#include "imagetablemodel.h"
#include "settings.h"

#include <QAreaSeries>
#include <QLineSeries>
#include <QValueAxis>
#include <QElapsedTimer>

HistogramChartView::HistogramChartView(QWidget *parent):
    ChartView (parent)
{

}

void HistogramChartView::drawSeries_()
{
    QVector<float> all_data;
    QVector<float> selected_data;
    for(int i=0;i<model_->rowCount();++i){
        QVariant val=model_->data(model_->index(i,active_column_),ImageTableModel::SortRole);
		bool export_flag=model_->data(model_->index(i,0),ImageTableModel::SortRole)==1;
        if(val.canConvert<float>() && val.toString()!=QString("")){
            float fval=val.toFloat();
            if(export_flag){
                selected_data.append(fval);
            }
            all_data.append(fval);
        }
    }
    Settings settings;
    int histogram_bins=settings.value("histogram_bins",256).toInt();
    auto chart_min_max = std::minmax_element(all_data.begin(), all_data.end());
    Histogram all(*chart_min_max.first,*chart_min_max.second,histogram_bins),selected(*chart_min_max.first,*chart_min_max.second,histogram_bins);
    all.add(all_data);
    selected.add(selected_data);
    QtCharts::QLineSeries *full_series = new QtCharts::QLineSeries();
    double half_gap=0.05;
    double width=all.width();
    double half_margin=width*(0.5-half_gap);
    foreach(QPointF p,all.dataPoints()){
        double pxl=p.x()-half_margin;
        double pxr=p.x()+half_margin;
        full_series->append(pxl,0.0);
        full_series->append(pxl,p.y());
        full_series->append(pxr,p.y());
        full_series->append(pxr,0);
    }
    full_series->setColor(color_);
    QtCharts::QAreaSeries *aseries = new QtCharts::QAreaSeries(full_series);
    aseries->setBrush(QBrush(color_));
    aseries->setPen(Qt::NoPen);
    chart()->addSeries(aseries);

    QtCharts::QLineSeries *selected_series = new QtCharts::QLineSeries();
    foreach(QPointF p,selected.dataPoints()){
        double pxl=p.x()-half_margin;
        double pxr=p.x()+half_margin;
        selected_series->append(pxl,0.0);
        selected_series->append(pxl,p.y());
        selected_series->append(pxr,p.y());
        selected_series->append(pxr,0);
    }
    selected_series->setColor(selected_color_);
    QtCharts::QAreaSeries *selected_aseries = new QtCharts::QAreaSeries(selected_series);
    selected_aseries->setBrush(QBrush(selected_color_));
    selected_aseries->setPen(Qt::NoPen);
    chart()->addSeries(selected_aseries);
}

void HistogramChartView::deselectData_(float start, float end, bool invert)
{
    if(invert){
        for(int i=0;i<model_->rowCount();++i){
            QVariant val=model_->data(model_->index(i,active_column_),ImageTableModel::SortRole);
            if(val.canConvert<float>() && val.toString()!=QString("")){
                float fval=val.toFloat();
                if(fval>=start && fval<=end){
                    model_->setData(model_->index(i,0),Qt::Unchecked,Qt::CheckStateRole);
                }
            }
        }
    }else{
        for(int i=0;i<model_->rowCount();++i){
            QVariant val=model_->data(model_->index(i,active_column_),ImageTableModel::SortRole);
            Data data=model_->image(i);
            if(val.canConvert<float>() && val.toString()!=QString("")){
                float fval=val.toFloat();
                if(fval<start || fval>end){
                    model_->setData(model_->index(i,0),Qt::Unchecked,Qt::CheckStateRole);
                }
            }
        }
    }
}

void HistogramChartView::mouseReleaseEvent(QMouseEvent *event)
{
    ChartView::mouseReleaseEvent(event);
    if(chart()->axes(Qt::Vertical).size()>0){
        QtCharts::QValueAxis* y_axis=dynamic_cast<QtCharts::QValueAxis*>(chart()->axes(Qt::Vertical)[0]);
        if(y_axis->min()<0){
            y_axis->setMin(0);
        }

    }
}
