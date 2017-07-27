#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QHash>
#include <QMainWindow>
#include <QSortFilterProxyModel>
#include <imageprocessor.h>
#include <imagetablemodel.h>

namespace Ui {
class MainWindow;
}

//fw decl
class QLabel;

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
    void onDestinationDirBrowse();
    void onStartStop(bool start);
    void addImage(const DataPtr &data);
    void onDataChanged(const DataPtr &data);
    void onAvgSourceDirTextChanged(const QString & dir);
    void onStackSourceDirTextChanged(const QString & dir);
    void onDestinationDirTextChanged(const QString & dir);
    void updateDetailsfromModel(const QModelIndex & topLeft, const QModelIndex & bottomRight);
    void updateDetailsfromView(const QModelIndex & topLeft, const QModelIndex & bottomRight);
    void onSettings();
    void inputDataChanged();
    void onExport();
    void updateQueueCounts(int cpu_queue, int gpu_queue);
signals:
    void startStop(bool start);
    void settingsChanged();
    void exportImages(const QString& path, QStringList& images);

private:
    void updateDetails_(int row);
    void updateTaskWidget_(QSettings *settings);
    Ui::MainWindow *ui;
    ImageTableModel *model_;
    QSortFilterProxyModel *sort_proxy_;
    QLabel *statusbar_queue_count_;
};

#endif // MAINWINDOW_H
