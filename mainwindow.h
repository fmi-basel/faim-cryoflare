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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void init();
public slots:
    void onAvgSourceDirBrowse();
    void onStackSourceDirBrowse();
    void onDestinationDirBrowse();
    void onStartStop(bool start);
    void addImage(const DataPtr &data);
    void onDataChanged(const DataPtr &data);
    void onTasksChanged(const TaskPtr & root);
    void onAvgSourceDirTextChanged(const QString & dir);
    void onStackSourceDirTextChanged(const QString & dir);
    void onDestinationDirTextChanged(const QString & dir);
signals:
    void avgSourceDirChanged(QString);
    void stackSourceDirChanged(QString);
    void destinationDirChanged(QString);
    void startStop(bool start);

private:
    Ui::MainWindow *ui;
    ImageTableModel *model_;
    QSortFilterProxyModel *sort_proxy_;
};

#endif // MAINWINDOW_H
