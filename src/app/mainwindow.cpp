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

#include <cmath>
#include <limits>
#include "processindicator.h"
#include "processwrapper.h"
#include "metadatastore.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "epuimageinfo.h"
#include "pathedit.h"
#include <QtDebug>
#include <QFileDialog>
#include "settings.h"
#include <QGroupBox>
#include <QFormLayout>
#include <settingsdialog.h>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QScrollArea>
#include <QToolTip>
#include <QElapsedTimer>
#include <QTextTable>
#include <QInputDialog>
#include "scatterplotdialog.h"
#include "aboutdialog.h"
#include "exportdialog.h"

MainWindow::MainWindow(MetaDataStore* meta_data_store, MicrographProcessor *processor, TaskConfiguration *task_configuration) :
    QMainWindow(),
    meta_data_store_(meta_data_store),
    processor_(processor),
    ui(new Ui::MainWindow),
    statusbar_queue_count_(new QLabel("CPU queue: 0 / GPU queue: 0")),
    process_indicators_(),
    epu_disk_usage_(new DiskUsageWidget("EPU")),
    movie_disk_usage_(new DiskUsageWidget("Movies")),
    local_disk_usage_(new DiskUsageWidget("Local")),
    last_image_timer_(new LastImageTimer()),
    task_configuration_(task_configuration)
{
    ui->setupUi(this);
    ui->avg_source_dir->setPathType(PathEdit::ExistingDirectory);
    ui->stack_source_dir->setPathType(PathEdit::ExistingDirectory);
    ui->avg_source_dir->setIconSize(32);
    ui->stack_source_dir->setIconSize(32);
    connect(this, SIGNAL(startStop(bool)), processor_, SLOT(startStop(bool)));
    connect(this,SIGNAL(settingsChanged()),processor_,SLOT(loadSettings()));
    connect(processor_,&MicrographProcessor::queueCountChanged,this,&MainWindow::updateQueueCounts);
    connect(processor_,&MicrographProcessor::processesCreated,this,&MainWindow::createProcessIndicators);
    connect(processor_,&MicrographProcessor::processesDeleted,this,&MainWindow::deleteProcessIndicators);
    connect(ui->avg_source_dir,&PathEdit::pathChanged,this,&MainWindow::onProjectPathChanged);
    connect(ui->stack_source_dir,&PathEdit::pathChanged,this,&MainWindow::onMoviePathChanged);
    connect(ui->tab_widget,&QTabWidget::currentChanged,this,&MainWindow::currentTabChanged);
    QString stylesheet;
    stylesheet+="* {color: #e6e6e6; background-color: #40434a} ";
    stylesheet+=" QScrollBar:vertical { width: 15px; margin: 15px 0px 15px 0px;} QScrollBar::handle:vertical {border: 1px solid #e6e6e6 ; background-color: rgb(5,97,137)}  QScrollBar::add-line:vertical { height: 15px; subcontrol-position: bottom; subcontrol-origin: margin;} QScrollBar::sub-line:vertical {height: 15px; subcontrol-position: top; subcontrol-origin: margin;}";
    stylesheet+=" QScrollBar:horizontal { height: 15px; margin: 0px 15px 0px 15px;} QScrollBar::handle:horizontal {border: 1px solid #e6e6e6 ; background-color: rgb(5,97,137)}  QScrollBar::add-line:horizontal { width: 15px; subcontrol-position: right; subcontrol-origin: margin;} QScrollBar::sub-line:horizontal {width: 15px; subcontrol-position: left; subcontrol-origin: margin;}";
    stylesheet+="QLineEdit{background-color: rgb(136, 138, 133)} ";
    stylesheet+="QLineEdit:disabled{background-color: rgb(80, 80, 80)} ";
    stylesheet+="QGraphicsView {padding:0px;margin:0px; border: 1px; border-radius: 5px; background: qlineargradient(x1:0, y1:0, x2:0, y2:1,stop:0 rgb(5,97,137), stop:1 rgb(16,27,50))} ";
    qApp->setStyleSheet(stylesheet);
    connect(ui->start_stop, SIGNAL(toggled(bool)), this, SLOT(onStartStopButton(bool)));
    statusBar()->addPermanentWidget(last_image_timer_);
    connect(meta_data_store,&MetaDataStore::newMicrograph,last_image_timer_,&LastImageTimer::reset);
    statusBar()->addPermanentWidget(epu_disk_usage_);
    statusBar()->addPermanentWidget(movie_disk_usage_);
    statusBar()->addPermanentWidget(local_disk_usage_);
    statusBar()->addPermanentWidget(statusbar_queue_count_);

    QMenu *tools_menu=new QMenu("Tools",this);
    QAction* scatter_plot_action=new QAction("Scatter Plot",this);
    tools_menu->addAction(scatter_plot_action);
    connect(scatter_plot_action, &QAction::triggered, this, &MainWindow::displayScatterPlot);
    menuBar()->addMenu(tools_menu);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    Settings settings;
    ui->avg_source_dir->setPath(settings.value("avg_source_dir").toString());
    ui->stack_source_dir->setPath(settings.value("stack_source_dir").toString());
    ui->micrographs->init(this,meta_data_store_,processor_,task_configuration_);
    ui->gridsquares->init(meta_data_store_,task_configuration_);
    QMenu *help_menu=new QMenu("Help",this);
    QAction* about_action=help_menu->addAction("About");
    connect(about_action,&QAction::triggered,this, &MainWindow::showAbout);
    menuBar()->addMenu(help_menu);

}


void MainWindow::writeReport()
{
    QStringList items;
    items << "PDF Report" << "CSV" << "CSV filtered" << "JSON"<< "JSON filtered";
    bool ok;
    QString item = QInputDialog::getItem(this, "Create report","Report type:", items, 0, false, &ok);
    QString file_types;
    if (ok && !item.isEmpty()){
        if(item.startsWith("PDF")){
            file_types="Pdf files (*.pdf)";
        }else if (item.startsWith("CSV")) {
            file_types="CSV files (*.csv)";
        }else{
            file_types="JSON files (*.json)";
        }
        QString file_name = QFileDialog::getSaveFileName(nullptr, "Save Report",".",file_types);
        if (!file_name.isEmpty()){
            meta_data_store_->createReport(file_name,item);
        }
    }
}

void MainWindow::onProjectPathChanged(const QString &/*dir*/)
{
    Settings settings;
    settings.setValue("avg_source_dir",ui->avg_source_dir->path());
    settings.saveToFile(CRYOFLARE_INI, QStringList(), QStringList() << "avg_source_dir");
}

void MainWindow::onMoviePathChanged(const QString &/*dir*/)
{
    Settings settings;
    settings.setValue("stack_source_dir",ui->stack_source_dir->path());
    settings.saveToFile(CRYOFLARE_INI, QStringList(), QStringList() << "stack_source_dir");
}

void MainWindow::onSettings()
{
    SettingsDialog settings_dialog(task_configuration_, this);
    if (QDialog::Accepted==settings_dialog.exec()){
        settings_dialog.saveSettings();
        Settings settings;
        settings.saveToFile(CRYOFLARE_INI);
        emit settingsChanged();
    }
}


void MainWindow::onExport()
{
    Settings settings;
    ExportDialog dialog(meta_data_store_->rawKeys().toList(),meta_data_store_->outputKeys().toList(),meta_data_store_->sharedRawKeys().toList(),meta_data_store_->sharedKeys().toList());
    dialog.setDuplicateRaw(settings.value("duplicate_raw_export").toBool());
    dialog.setSeparateRawPath(settings.value("separate_raw_export").toBool());
    dialog.setDestinationPath(QUrl::fromUserInput(settings.value("export_path").toString()));
    if(settings.value("separate_raw_export").toBool()){
        dialog.setRawDestinationPath(QUrl::fromUserInput(settings.value("raw_export_path").toString()));
    }
    if(dialog.exec()==QDialog::Accepted){
        settings.setValue("export_path",dialog.destinationPath().toString(QUrl::RemovePassword));
        settings.setValue("separate_raw_export",dialog.separateRawPath());
        settings.setValue("duplicate_raw_export",dialog.duplicateRaw());
        if(dialog.separateRawPath()){
            settings.setValue("raw_export_path",dialog.rawDestinationPath().toString(QUrl::RemovePassword));
        }
        settings.saveToFile(CRYOFLARE_INI, QStringList(), QStringList() << "export_path" << "raw_export_path" << "separate_raw_export" << "duplicate_raw_export");
        meta_data_store_->exportMicrographs(dialog.destinationPath(),dialog.rawDestinationPath(),dialog.selectedOutputKeys(),dialog.selectedRawKeys(),dialog.selectedSharedKeys(),dialog.selectedSharedRawKeys(),dialog.duplicateRaw());
    }
}

void MainWindow::updateQueueCounts(int cpu_queue, int gpu_queue)
{
    statusbar_queue_count_->setText(QString("CPU queue: %1 / GPU queue: %2").arg(cpu_queue).arg(gpu_queue));
}




void MainWindow::createProcessIndicators(QList<ProcessWrapper *> wrappers)
{
    foreach(ProcessWrapper* w, wrappers){
        ProcessIndicator *indicator=new ProcessIndicator(w->gpuID());
        process_indicators_.append(indicator);
        statusBar()->addWidget(indicator);
        connect(w,&ProcessWrapper::started,indicator,&ProcessIndicator::started);
        connect(w,&ProcessWrapper::finished,indicator,&ProcessIndicator::finished);
    }
}

void MainWindow::deleteProcessIndicators()
{
    while(!process_indicators_.empty()){
        ProcessIndicator *indicator=process_indicators_.takeLast();
        statusBar()->removeWidget(indicator);
        indicator->deleteLater();
    }

}



void MainWindow::onStartStopButton(bool start)
{
    if(start){
        epu_disk_usage_->start(ui->avg_source_dir->path());
        movie_disk_usage_->start(ui->stack_source_dir->path());
        local_disk_usage_->start(".");
        last_image_timer_->reset();
    }else{
        last_image_timer_->stop();
    }
    emit startStop(start);
}

void MainWindow::showAbout()
{
    AboutDialog dialog;
    dialog.exec();
}



void MainWindow::displayScatterPlot()
{
    ScatterPlotDialog dialog(meta_data_store_, task_configuration_);
    dialog.exec();
}

void MainWindow::currentTabChanged(int idx)
{
    for(int i=0;i<ui->tab_widget->count();++i){
        ui->tab_widget->widget(i)->setEnabled(idx==i);
    }
}
