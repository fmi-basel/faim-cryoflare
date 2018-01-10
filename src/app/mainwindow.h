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

namespace Ui {
class MainWindow;
}

//fw decl
class QLabel;
class Settings;
class QChart;
class QFormLayout;
class QVBoxLayout;

struct ChartData
{
    ChartData():
        line_list(),
        histogram()

    {}
    ChartData(const QList<QList<QPointF> >& l, const QList<QPointF>&  h):
        line_list(l),
        histogram(h)
    {}
    QList<QList<QPointF> > line_list;
    QList<QPointF> histogram;
};

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
    void onStartStop(bool start);
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

signals:
    void startStop(bool start);
    void settingsChanged();
    void exportImages(const QString& path,const QStringList& images);

private:
    void updateTaskWidget_(Settings *settings, QFormLayout *parent_input_layout, QFormLayout *parent_output_layout);
    Ui::MainWindow *ui;
    ImageTableModel *model_;
    QSortFilterProxyModel *sort_proxy_;
    QLabel *statusbar_queue_count_;
    QTimer chart_update_timer_;
};

#endif // MAINWINDOW_H
