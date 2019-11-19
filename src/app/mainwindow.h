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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QHash>
#include <QMainWindow>
#include "imagetablesortfilterproxymodel.h"
#include <QFuture>
#include <QFutureWatcher>
#include <QPair>
#include <QChart>
#include <QTimer>
#include <micrographprocessor.h>
#include <imagetablemodel.h>
#include "positionchart.h"
#include "tablesummarymodel.h"
#include "micrographprocessor.h"
#include "diskusagewidget.h"
#include "lastimagetimer.h"
#include "gridsquaretablemodel.h"

namespace Ui {
class MainWindow;
}

//fw decl
class QLabel;
class Settings;
class QFormLayout;
class QVBoxLayout;
class ProcessIndicator;
class ExportProgressDialog;
class MetaDataStore;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(MetaDataStore* meta_data_store,MicrographProcessor* processor, TaskConfiguration* task_configuration);
    ~MainWindow();
    void init();
public slots:
    void onProjectPathChanged(const QString & dir);
    void onMoviePathChanged(const QString & dir);
    void onSettings();
    void onExport();
    void updateQueueCounts(int cpu_queue, int gpu_queue);
    void createProcessIndicators(QList<ProcessWrapper *> wrappers);
    void deleteProcessIndicators();
    void writeReport();
    void onStartStopButton(bool start);
    void showAbout();
    void displayScatterPlot();
    void currentTabChanged(int idx);


signals:
    void startStop(bool start);
    void settingsChanged();
    void cancelExport();

private:
    MetaDataStore*  meta_data_store_;
    MicrographProcessor* processor_;
    Ui::MainWindow *ui;
    QLabel *statusbar_queue_count_;
    QList<ProcessIndicator*> process_indicators_;
    DiskUsageWidget* epu_disk_usage_;
    DiskUsageWidget* movie_disk_usage_;
    DiskUsageWidget* local_disk_usage_;
    LastImageTimer* last_image_timer_;
    TaskConfiguration* task_configuration_;
};

#endif // MAINWINDOW_H
