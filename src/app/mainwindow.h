#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QHash>
#include <QMainWindow>
#include <QSortFilterProxyModel>
#include <QFuture>
#include <QFutureWatcher>
#include <QPair>
#include <QChart>
#include <QTimer>
#include <imageprocessor.h>
#include <imagetablemodel.h>
#include "positionchart.h"

namespace Ui {
class MainWindow;
}

//fw decl
class QLabel;
class Settings;
class QChart;
class QFormLayout;
class QVBoxLayout;
class ProcessIndicator;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
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
    void exportLinearChart();
    void exportHistogramChart();
    void selectFromLinearChart(float start, float end, bool invert);
    void selectFromHistogramChart(float start, float end, bool invert);
    void onStartStopButton(bool start);
    void showAbout();
    void phasePlateClicked(int n);
    void phasePlateBack();
    void gridSquareClicked(int n);
    void gridSquareBack();

signals:
    void startStop(bool start);
    void settingsChanged();
    void exportImages(const QString& path,const QStringList& images);

private slots:

private:
    void updateTaskWidget_(Settings *settings, QFormLayout *parent_input_layout, QFormLayout *parent_output_layout);
    Ui::MainWindow *ui;
    ImageTableModel *model_;
    QSortFilterProxyModel *sort_proxy_;
    QLabel *statusbar_queue_count_;
    QTimer chart_update_timer_;
    QList<ProcessIndicator*> process_indicators_;
    float histogram_min_;
    float histogram_bucket_size_;
    QVector<float> histogram_;
    PositionChart* phase_plate_chart_;
    PositionChart* phase_plate_position_chart_;
    PositionChart* phase_plate_content_chart_;
    int phase_plate_level_;
    PositionChart* grid_square_chart_;
    PositionChart* grid_square_position_chart_;
    PositionChart* grid_square_content_chart_;
    int grid_square_level_;
};

#endif // MAINWINDOW_H
