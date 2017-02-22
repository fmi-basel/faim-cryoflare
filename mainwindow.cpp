#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "epuimageinfo.h"
#include <QtDebug>
#include <QFileDialog>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    model_(new ImageTableModel(this)),
    sort_proxy_(new QSortFilterProxyModel(this))
{
    ui->setupUi(this);
    sort_proxy_->setSourceModel(model_);
    ui->image_list->setModel(sort_proxy_);
    //ui->image_list->horizontalHeader()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    connect(ui->avg_source_dir, SIGNAL(textChanged(QString)), this, SIGNAL(avgSourceDirChanged(QString)));
    connect(ui->stack_source_dir, SIGNAL(textChanged(QString)), this, SIGNAL(stackSourceDirChanged(QString)));
    connect(ui->destination_dir, SIGNAL(textChanged(QString)), this, SIGNAL(destinationDirChanged(QString)));
    connect(ui->start_stop, SIGNAL(toggled(bool)), this, SLOT(onStartStop(bool)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    QSettings settings;
    ui->avg_source_dir->setText(settings.value("avg_source_dir").toString());
    ui->stack_source_dir->setText(settings.value("stack_source_dir").toString());
    ui->destination_dir->setText(settings.value("destination_dir").toString());
}

void MainWindow::onAvgSourceDirBrowse()
{
    QString dir_name = QFileDialog::getExistingDirectory(this, tr("Open Directory"),"",QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);
    if(!dir_name.isEmpty()){
        ui->avg_source_dir->setText(dir_name);
    }
}

void MainWindow::onStackSourceDirBrowse()
{
    QString dir_name = QFileDialog::getExistingDirectory(this, tr("Open Directory"),"",QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);
    if(!dir_name.isEmpty()){
        ui->stack_source_dir->setText(dir_name);
    }
}

void MainWindow::onDestinationDirBrowse()
{
    QString dir_name = QFileDialog::getExistingDirectory(this, tr("Open Directory"),"",QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);
    if(!dir_name.isEmpty()){
        ui->destination_dir->setText(dir_name);
    }
}

void MainWindow::onStartStop(bool start)
{
    if(start){
        ui->start_stop->setText("Stop");
    }else{
        ui->start_stop->setText("Start");
    }
    emit startStop(start);
}

void MainWindow::addImage(const DataPtr &data)
{
    model_->addImage(data);
}

void MainWindow::onDataChanged(const DataPtr &data)
{
    model_->onDataChanged(data);
}

void MainWindow::onTasksChanged(const TaskPtr &root)
{
    model_->setColumns(root->getDisplayKeys());
}


void MainWindow::onAvgSourceDirTextChanged(const QString &dir)
{
    QSettings settings;
    settings.setValue("avg_source_dir",ui->avg_source_dir->text());
    emit avgSourceDirChanged(dir);
}

void MainWindow::onStackSourceDirTextChanged(const QString &dir)
{
    QSettings settings;
    settings.setValue("stack_source_dir",ui->stack_source_dir->text());
    emit stackSourceDirChanged(dir);
}

void MainWindow::onDestinationDirTextChanged(const QString &dir)
{
    QSettings settings;
    settings.setValue("destination_dir",ui->destination_dir->text());
    emit destinationDirChanged(dir);
}
