//
// This file is part of CryoFLARE
//
// Copyright (C) 2019 by the CryoFLARE Authors
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
#include "scatterchartview.h"
#include <QtCharts/QScatterSeries>
#include "imagetablemodel.h"

ScatterChartView::ScatterChartView(QWidget *parent):
    ChartView(parent),
    x_column_(0)
{

}

void ScatterChartView::setXColumn(int col)
{
    x_column_=col;
}

void ScatterChartView::drawChart_()
{
    ChartView::drawChart_();
    chart()->axisX(chart()->series().first())->setTitleText(model_->headerData(x_column_,Qt::Horizontal,Qt::DisplayRole).toString());
    chart()->axisY(chart()->series().first())->setTitleText(model_->headerData(active_column_,Qt::Horizontal,Qt::DisplayRole).toString());
    chart()->setTitle(QString("%1 / %2").arg(model_->headerData(active_column_,Qt::Horizontal,Qt::DisplayRole).toString()).arg(model_->headerData(x_column_,Qt::Horizontal,Qt::DisplayRole).toString()));
}

void ScatterChartView::drawSeries_()
{
    QtCharts::QScatterSeries *selected = new QtCharts::QScatterSeries();
    QtCharts::QScatterSeries *deselected = new QtCharts::QScatterSeries();
    selected->setMarkerSize(6);
    selected->setPen(QPen(QBrush(Qt::white),1));
    selected->setColor(selected_color_);
    deselected->setMarkerSize(6);
    deselected->setPen(QPen(QBrush(Qt::white),1));
    deselected->setColor(color_);
    for(int i=0;i<model_->rowCount();++i){
        QVariant yval=model_->data(model_->index(i,active_column_),ImageTableModel::SortRole);
        QVariant xval=model_->data(model_->index(i,x_column_),ImageTableModel::SortRole);
        Data data=model_->image(i);
        QString export_val=data.value("export").toString("true");
        bool export_flag=export_val.compare("true", Qt::CaseInsensitive) == 0 || export_val==QString("1") ;
        if(xval.canConvert<float>() && yval.canConvert<float>() && xval.toString()!=QString("") && yval.toString()!=QString("")){
            qreal fxval=xval.toDouble();
            qreal fyval=yval.toDouble();
            if(export_flag){
                selected->append(fxval,fyval);
            }else{
                deselected->append(fxval,fyval);
            }
        }
    }
    chart()->addSeries(selected);
    chart()->addSeries(deselected);
}


