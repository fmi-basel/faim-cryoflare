//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
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
#include <QChart>
#include <QGraphicsLayout>
#include <QtCharts/QScatterSeries>
#include "scatterplotdialog.h"
#include "ui_scatterplotdialog.h"
#include "imagetablemodel.h"

ScatterPlotDialog::ScatterPlotDialog(MetaDataStore * store, TaskConfiguration *task_config, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScatterPlotDialog)
{
    ui->setupUi(this);
    ui->chart->chart()->setTheme(QtCharts::QChart::ChartThemeBlueCerulean);
    ui->chart->setRenderHint(QPainter::Antialiasing);
    ui->chart->chart()->layout()->setContentsMargins(0,0,0,0);
    QStringList columns;
    ui->chart->setModel(new ImageTableModel(store,task_config, this));
    foreach(InputOutputVariable v, task_config->resultLabels()){
        columns << v.key;
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
    ui->chart->setActiveColumn(ui->list_y->currentRow()+1);
    ui->chart->setXColumn(ui->list_x->currentRow()+1);
}
