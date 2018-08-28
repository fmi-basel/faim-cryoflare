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
#include <imageprocessor.h>
#include <imagetablemodel.h>
#include "positionchart.h"
#include "tablesummarymodel.h"
#include "imageprocessor.h"
#include <LimeReport>

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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(ImageProcessor& processor);
    ~MainWindow();
    void init();
    void updateTaskWidgets();
public slots:
    void onAvgSourceDirBrowse();
    void onStackSourceDirBrowse();
    void addImage(const DataPtr &data);
    void onDataChanged(const DataPtr &data);
    void onAvgSourceDirTextChanged(const QString & dir);
    void onStackSourceDirTextChanged(const QString & dir);
    void updateDetailsfromModel(const QModelIndex & topLeft, const QModelIndex & bottomRight);
    void updateDetailsfromView(const QModelIndex & topLeft, const QModelIndex & bottomRight);
    void onSettings();
    void inputDataChanged();
    void onExport();
    void updateQueueCounts(int cpu_queue, int gpu_queue);
    void updateDetails();
    void updateChart();
    void updatePhasePlateChart();
    void updateGridSquareChart();
    void createProcessIndicator(ProcessWrapper * wrapper, int gpu_id);
    void deleteProcessIndicators();
    void displayLinearChartDetails(const QPointF &point, bool state);
    void displayHistogramChartDetails(const QPointF &point, bool state);
    void writeReport();
    void selectFromLinearChart(float start, float end, bool invert);
    void selectFromHistogramChart(float start, float end, bool invert);
    void onStartStopButton(bool start);
    void showAbout();
    void phasePlateSelectionChanged();
    void phasePlateSelectionFinished(QRect rubberBandRect, QPointF fromScenePoint, QPointF toScenePoint);
    void gridSquareSelectionChanged();
    void gridSquareSelectionFinished(QRect rubberBandRect, QPointF fromScenePoint, QPointF toScenePoint);
    void displayScatterPlot();
    void enableSelection(bool selecting);
    void onExportStarted(const QString& message, int num_files);
    void onExportMessage(int left, const QList<ExportMessage>& messages);
    void onExportFinished();

signals:
    void startStop(bool start);
    void settingsChanged();
    void cancelExport();
private slots:

private:
    void updateTaskWidget_(Settings *settings, QFormLayout *parent_input_layout, QFormLayout *parent_output_layout);
    ImageProcessor& processor_;
    Ui::MainWindow *ui;
    ImageTableModel *model_;
    //used for report builder: TODO merge with model_
    ImageTableModel *full_model_;
    ImageTableSortFilterProxyModel *sort_proxy_;
    TableSummaryModel* summary_model_;
    QLabel *statusbar_queue_count_;
    QTimer chart_update_timer_;
    QList<ProcessIndicator*> process_indicators_;
    float histogram_min_;
    float histogram_bucket_size_;
    QVector<float> histogram_;
    PositionChart* phase_plate_chart_;
    PositionChart* phase_plate_position_chart_;
    int phase_plate_level_;
    int current_phase_plate_;
    int chart_current_square_;
    QList<InputOutputVariable> default_columns_;
    QAction* scatter_plot_action_;
    QAction* run_script_action_;
    LimeReport::ReportEngine report_;
    ExportProgressDialog* export_progress_dialog_;

};

#endif // MAINWINDOW_H
