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
#include <QChart>
#include <QGraphicsLayout>
#include <QtCharts/QScatterSeries>
#include "scatterplotdialog.h"
#include "ui_scatterplotdialog.h"
#include "imagetablemodel.h"

ScatterPlotDialog::ScatterPlotDialog(MetaDataStore * store, TaskConfiguration *task_config, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScatterPlotDialog),
    col_ids_()
{
    ui->setupUi(this);
    ui->chart->chart()->setTheme(QtCharts::QChart::ChartThemeBlueCerulean);
    ui->chart->setRenderHint(QPainter::Antialiasing);
    ui->chart->chart()->layout()->setContentsMargins(0,0,0,0);
    QStringList columns;
    ui->chart->setModel(new ImageTableModel(store,task_config, this));
    int col_id=1;
    foreach(InputOutputVariable v, task_config->resultLabels()){
        if(v.in_column){
            columns << v.key;
            col_ids_.append(col_id);
        }
        ++col_id;
    }
    ui->list_x->addItems(columns);
    ui->list_x->setCurrentRow(0);
    connect(ui->list_x,&QListWidget::currentRowChanged,this,&ScatterPlotDialog::updateChart);
    ui->list_y->addItems(columns);
    ui->list_y->setCurrentRow(0);
    connect(ui->list_y,&QListWidget::currentRowChanged,this,&ScatterPlotDialog::updateChart);
}

ScatterPlotDialog::~ScatterPlotDialog()
{
    delete ui;
}

void ScatterPlotDialog::updateChart()
{
    ui->chart->setActiveColumn(col_ids_.at(ui->list_y->currentRow()));
    ui->chart->setXColumn(col_ids_.at(ui->list_x->currentRow()));
}
