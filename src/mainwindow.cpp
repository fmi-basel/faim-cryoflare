#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "epuimageinfo.h"
#include <QtDebug>
#include <QFileDialog>
#include <QSettings>
#include <QGroupBox>
#include <QFormLayout>
#include <settings.h>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QScrollArea>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    model_(new ImageTableModel(this)),
    sort_proxy_(new QSortFilterProxyModel(this)),
    statusbar_queue_count_(new QLabel("CPU queue: 0 / GPU queue: 0"))
{
    ui->setupUi(this);
    sort_proxy_->setSourceModel(model_);
    sort_proxy_->setSortRole(ImageTableModel::SortRole);
    ui->image_list->setModel(sort_proxy_);
    //ui->image_list->horizontalHeader()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    connect(ui->avg_source_dir, SIGNAL(textChanged(QString)), this, SLOT(onAvgSourceDirTextChanged(QString)));
    connect(ui->stack_source_dir, SIGNAL(textChanged(QString)), this, SLOT(onStackSourceDirTextChanged(QString)));
    connect(ui->destination_dir, SIGNAL(textChanged(QString)), this, SLOT(onDestinationDirTextChanged(QString)));
    connect(ui->start_stop, SIGNAL(toggled(bool)), this, SLOT(onStartStop(bool)));
    connect(model_,SIGNAL(dataChanged(QModelIndex,QModelIndex)),this,SLOT(updateDetailsfromModel(QModelIndex,QModelIndex)));
    connect(ui->image_list->selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(updateDetailsfromView(QModelIndex,QModelIndex)));
    statusBar()->addPermanentWidget(statusbar_queue_count_);
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
    updateTaskWidgets();
}

void MainWindow::onAvgSourceDirBrowse()
{
    QString dir_name = QFileDialog::getExistingDirectory(this, tr("Open Directory"),ui->avg_source_dir->text(),QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);
    if(!dir_name.isEmpty()){
        ui->avg_source_dir->setText(dir_name);
    }
}

void MainWindow::onStackSourceDirBrowse()
{
    QString dir_name = QFileDialog::getExistingDirectory(this, tr("Open Directory"),ui->stack_source_dir->text(),QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);
    if(!dir_name.isEmpty()){
        ui->stack_source_dir->setText(dir_name);
    }
}

void MainWindow::onDestinationDirBrowse()
{
    QString dir_name = QFileDialog::getExistingDirectory(this, tr("Open Directory"),ui->destination_dir->text(),QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);
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

void MainWindow::updateTaskWidgets()
{
    for(int i=ui->image_data->count()-1;i>=0;--i){
        QWidget *widget_ptr=ui->image_data->widget(i);
        ui->image_data->removeTab(i);
        delete widget_ptr;
    }
    model_->clearColumns();
    QSettings *settings=new QSettings;
    model_->addColumn(InputOutputVariable("Name","short_name",String));
    model_->addColumn(InputOutputVariable("Timestamp","timestamp",String));
    model_->addColumn(InputOutputVariable("Nominal Defocus","defocus",Float));
    model_->addColumn(InputOutputVariable("Exposure time","exposure_time",Float));
    model_->addColumn(InputOutputVariable("Pixel size","apix_x",Float));
    model_->addColumn(InputOutputVariable("Number of Frames","num_frames",Int));
    settings->beginGroup("Tasks");
    updateTaskWidget_(settings);
    settings->endGroup();
    delete settings;
}

void MainWindow::updateTaskWidget_(QSettings *settings)
{

    foreach(QString child_name, settings->childGroups()){
        settings->beginGroup(child_name);
        QWidget* widget=new QWidget();
        QScrollArea* scroll_area=new QScrollArea();
        QVBoxLayout *layout = new QVBoxLayout;
        QGroupBox *input_group=new QGroupBox("Input");
        layout->addWidget(input_group);
        QFormLayout *input_layout = new QFormLayout;
        input_group->setLayout(input_layout);
        QList<QVariant> variant_list=settings->value("input_variables").toList();
        QSettings script_input_settings;
        foreach(QVariant v,variant_list){
            InputOutputVariable iov(v);
            QWidget* local_widget;
            if(iov.type==Image || iov.type==String){
                QLineEdit *le_widget=new QLineEdit();
                le_widget->setText(script_input_settings.value("ScriptInput/"+child_name+"/"+iov.label).toString());
                local_widget=le_widget;
                connect(local_widget,SIGNAL(textChanged(QString)),this,SLOT(inputDataChanged()));
            }else if(iov.type==Int){
                QSpinBox *sp_widget=new QSpinBox();
                sp_widget->setMaximum(9999999);
                local_widget=sp_widget;
                sp_widget->setValue(script_input_settings.value("ScriptInput/"+child_name+"/"+iov.label).toInt());
                connect(local_widget,SIGNAL(valueChanged(int)),this,SLOT(inputDataChanged()));
            }else if(iov.type==Float){
                QDoubleSpinBox *sp_widget=new QDoubleSpinBox();
                sp_widget->setMaximum(9999999);
                local_widget=sp_widget;
                sp_widget->setValue(script_input_settings.value("ScriptInput/"+child_name+"/"+iov.label).toFloat());
                connect(local_widget,SIGNAL(valueChanged(double)),this,SLOT(inputDataChanged()));
            }
            input_layout->addRow(iov.key,local_widget);
            local_widget->setProperty("type",iov.type);
            local_widget->setProperty("label",iov.label);
            local_widget->setProperty("task",child_name);

        }
        QGroupBox *output_group=new QGroupBox("Output");
        layout->addWidget(output_group);
        QFormLayout *output_layout = new QFormLayout;
        output_group->setLayout(output_layout);
        variant_list=settings->value("output_variables").toList();
        foreach(QVariant v,variant_list){
            InputOutputVariable iov(v);
            if(iov.in_column){
                model_->addColumn(iov);
            }else{
                QLabel *label=new QLabel();
                label->setProperty("type",iov.type);
                label->setProperty("label",iov.label);
                switch(iov.type){
                    case String:
                    case Int:
                    case Float:
                        output_layout->addRow(iov.key,label);
                        break;
                    case Image:
                        output_layout->addRow(new QLabel(iov.key));
                        label->setFixedSize(500,500);
                        output_layout->addRow(label);
                        break;
                }
            }
        }
        layout->addStretch(1);
        widget->setLayout(layout);
        scroll_area->setWidget(widget);
        ui->image_data->addTab(scroll_area,child_name);
        updateTaskWidget_(settings);
        settings->endGroup();
    }
}

void MainWindow::onAvgSourceDirTextChanged(const QString &dir)
{
    QSettings settings;
    settings.setValue("avg_source_dir",ui->avg_source_dir->text());
}

void MainWindow::onStackSourceDirTextChanged(const QString &dir)
{
    QSettings settings;
    settings.setValue("stack_source_dir",ui->stack_source_dir->text());
}

void MainWindow::onDestinationDirTextChanged(const QString &dir)
{
    QSettings settings;
    settings.setValue("destination_dir",ui->destination_dir->text());
}

void MainWindow::updateDetailsfromModel(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    int current_row=sort_proxy_->mapToSource(ui->image_list->currentIndex()).row();
    if(topLeft.row()<=current_row && bottomRight.row()>=current_row){
        updateDetails_(current_row);
    }
}

void MainWindow::updateDetailsfromView(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    int current_row=sort_proxy_->mapToSource(ui->image_list->currentIndex()).row();
    updateDetails_(current_row);
}

void MainWindow::onSettings()
{
    Settings settings(this);
    if (QDialog::Accepted==settings.exec()){
        settings.saveSettings();
        updateTaskWidgets();
        emit settingsChanged();
    }
}

void MainWindow::inputDataChanged()
{
    QObject *sender_widget=sender();
    QVariant label= sender_widget->property("label");
    QVariant type= sender_widget->property("type");
    QVariant task= sender_widget->property("task");
    if(label.isValid() && type.isValid() && task.isValid()){
       QSettings settings;
        settings.beginGroup("ScriptInput");
        settings.beginGroup(task.toString());
        switch(static_cast<VariableType>(type.toInt())){
        case String:
        case Image:
            settings.setValue(label.toString(),qobject_cast<QLineEdit*>(sender_widget)->text());
            break;
        case Int:
            settings.setValue(label.toString(),qobject_cast<QSpinBox*>(sender_widget)->value());
            break;
        case Float:
            settings.setValue(label.toString(),qobject_cast<QDoubleSpinBox*>(sender_widget)->value());
            break;
        }
        settings.endGroup();
        settings.endGroup();
    }
}

void MainWindow::onExport()
{
    QString export_path = QFileDialog::getExistingDirectory(0, "Export folder","",  QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);
    if(! export_path.isEmpty()){
        QStringList images;
        for(int i=0;i<model_->rowCount();++i){
            DataPtr data=model_->image(i);
            QString export_val=data->value("export","true");
            if (export_val.compare("true", Qt::CaseInsensitive) == 0 || export_val==QString("1")){
                images<<data->value("short_name");
            }
        }
        emit exportImages(export_path,images);
    }
}

void MainWindow::updateQueueCounts(int cpu_queue, int gpu_queue)
{
    statusbar_queue_count_->setText(QString("CPU queue: %1 / GPU queue: %2").arg(cpu_queue).arg(gpu_queue));
}

void MainWindow::updateDetails_(int row)
{
    DataPtr data=model_->image(row);
    for(int i=0;i< ui->image_data->count();++i){
        QScrollArea *scroll_area=qobject_cast<QScrollArea*>(ui->image_data->widget(i));
        if(!scroll_area){
            qDebug() << "found no scroll area at tab: " << i;
            continue;
        }
        QWidget * widget_ptr=scroll_area->widget();
        foreach( QObject* child, widget_ptr->children()){
            QString classname=child->metaObject()->className();	
            if(classname!=QString("QGroupBox")){
                continue;
            }
            foreach( QObject* inner_child, child->children()){
                QLabel *qlabel=qobject_cast<QLabel*>(inner_child);
                QVariant label= inner_child->property("label");
                QVariant type=inner_child->property("type");
                if(!label.isValid() || ! type.isValid() || !qlabel){
                    continue;
                }
                if(static_cast<VariableType>(type.toInt())==Image){
                    QString path=data->value(label.toString());
                    if(QFileInfo(path).exists()){
                        qlabel->setPixmap(QPixmap(path));
                    } else {
                        QPixmap p(512,512);
                        p.fill();
                        qlabel->setPixmap(p);
                    }
                }
            }
        }
    }
}
