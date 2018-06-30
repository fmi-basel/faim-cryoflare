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

#include <cmath>
#include <limits>
#include "processindicator.h"
#include "processwrapper.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "epuimageinfo.h"
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
#include <QBarSet>
#include <QBarCategoryAxis>
#include <QtConcurrent/QtConcurrentRun>
#include <QBarSeries>
#include <QLineSeries>
#include <QAreaSeries>
#include <QImageReader>
#include <QPicture>
#include <QToolTip>
#include <QElapsedTimer>
#include <QPrinter>
#include <QPrintDialog>
#include <QGraphicsLayout>
#include "scatterplotdialog.h"
#include "aboutdialog.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    model_(new ImageTableModel(this)),
    sort_proxy_(new ImageTableSortFilterProxyModel(this)),
    summary_model_(new TableSummaryModel(model_,this)),
    statusbar_queue_count_(new QLabel("CPU queue: 0 / GPU queue: 0")),
    chart_update_timer_(),
    process_indicators_(),
    histogram_min_(0),
    histogram_bucket_size_(1),
    histogram_(),
    phase_plate_chart_(new PositionChart(this)),
    phase_plate_position_chart_(new PositionChart(this)),
    phase_plate_level_(0),
    current_phase_plate_(-1),
    chart_current_square_(-1),
    default_columns_(),
    scatter_plot_action_(new QAction("Scatter Plot",this)),
    run_script_action_(new QAction("Run script",this))


{
    ui->setupUi(this);
    QString stylesheet;
    stylesheet+="* {color: #e6e6e6; background-color: #40434a} ";
    stylesheet+="QScrollBar::handle {background-color: rgb(5,97,137) }  ";
    stylesheet+="QLineEdit{background-color: rgb(136, 138, 133)} ";
    stylesheet+="QGraphicsView {padding:0px;margin:0px; border: 1px; border-radius: 5px; background: qlineargradient(x1:0, y1:0, x2:0, y2:1,stop:0 rgb(5,97,137), stop:1 rgb(16,27,50))} ";
    qApp->setStyleSheet(stylesheet);
    ui->chart->chart()->setTheme(QtCharts::QChart::ChartThemeBlueCerulean);
    ui->chart->chart()->layout()->setContentsMargins(0,0,0,0);
    ui->histogram->chart()->setTheme(QtCharts::QChart::ChartThemeBlueCerulean);
    ui->histogram->chart()->layout()->setContentsMargins(0,0,0,0);
    ui->image_list_summary->setSibling(ui->image_list);
    ui->image_list_summary->setStyleSheet("QHeaderView::section { padding-left: 1 px}");
    ui->image_list->horizontalHeader()->setStyleSheet("QHeaderView::section { padding-left:  8 px}");

    ui->chart->setRenderHints(QPainter::HighQualityAntialiasing|QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform|QPainter::Antialiasing);
    ui->chart->setOptimizationFlag(QGraphicsView::DontSavePainterState);
    ui->chart->setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing);
    ui->histogram->setRenderHints(QPainter::HighQualityAntialiasing|QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform|QPainter::Antialiasing);
    ui->histogram->setOptimizationFlag(QGraphicsView::DontSavePainterState);
    ui->histogram->setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing);
    sort_proxy_->setSourceModel(model_);
    sort_proxy_->setSortRole(ImageTableModel::SortRole);
    ui->image_list->setModel(sort_proxy_);
    ui->image_list_summary->setModel(summary_model_);
    connect(ui->avg_source_dir, SIGNAL(textChanged(QString)), this, SLOT(onAvgSourceDirTextChanged(QString)));
    connect(ui->stack_source_dir, SIGNAL(textChanged(QString)), this, SLOT(onStackSourceDirTextChanged(QString)));
    connect(ui->start_stop, SIGNAL(toggled(bool)), this, SLOT(onStartStopButton(bool)));
    connect(model_,SIGNAL(dataChanged(QModelIndex,QModelIndex)),this,SLOT(updateDetailsfromModel(QModelIndex,QModelIndex)));
    connect(ui->image_list->selectionModel(),SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(updateDetailsfromView(QModelIndex,QModelIndex)));
    statusBar()->addPermanentWidget(statusbar_queue_count_);
    chart_update_timer_.setSingleShot(true);

    QPainterPath phase_plate_path;
    phase_plate_path.moveTo(-80,-40);
    phase_plate_path.arcTo(-120,-40,80,80,90,180);
    phase_plate_path.arcTo(40,-40,80,80,270,180);
    phase_plate_path.closeSubpath();
    QHash<int,QPointF> phase_plate_xy_list;
    phase_plate_xy_list[1]=QPointF(0,-150);
    phase_plate_xy_list[2]=QPointF(-150,-50);
    phase_plate_xy_list[3]=QPointF(150,-50);
    phase_plate_xy_list[4]=QPointF(-150,50);
    phase_plate_xy_list[5]=QPointF(150,50);
    phase_plate_xy_list[6]=QPointF(0,150);
    phase_plate_chart_->addPositions(phase_plate_path,phase_plate_xy_list);
    connect(phase_plate_chart_,&PositionChart::selectionChanged,this,&MainWindow::phasePlateSelectionChanged);
    ui->phase_plate->setScene(phase_plate_chart_);
    connect(ui->phase_plate,&PositionChartView::rubberBandChanged,this,&MainWindow::phasePlateSelectionFinished);
    PositionChart* grid_chart(new PositionChart(this));
    connect(grid_chart,&PositionChart::selectionChanged,this,&MainWindow::gridSquareSelectionChanged,Qt::QueuedConnection);
    ui->grid_square_chart->setScene(grid_chart);
    connect(ui->grid_square_chart,&PositionChartView::rubberBandChanged,this,&MainWindow::gridSquareSelectionFinished);

    QPainterPath phase_plate_pos_path;
    phase_plate_pos_path.moveTo(40,0);
    phase_plate_pos_path.arcTo(-40,-40,80,80,0,360);
    QHash<int,QPointF> phase_plate_pos_xy_list;
    int num_rows=4;
    int num_columns=19;
    for(int y=0;y<num_rows;++y){
        for(int x=0;x<num_columns;++x){
            phase_plate_pos_xy_list[y*num_columns+x]=QPointF(100*x,100*y);
        }
    }
    phase_plate_position_chart_->addPositions(phase_plate_pos_path,phase_plate_pos_xy_list,true);
    connect(phase_plate_position_chart_,&PositionChart::selectionChanged,this,&MainWindow::phasePlateSelectionChanged);



    connect(&chart_update_timer_, &QTimer::timeout, this, &MainWindow::updateChart);
    QMenu *tools_menu=new QMenu("Tools",this);
    tools_menu->addAction(scatter_plot_action_);
    tools_menu->addAction(run_script_action_);
    connect(scatter_plot_action_, &QAction::triggered, this, &MainWindow::displayScatterPlot);
     menuBar()->addMenu(tools_menu);
    QMenu *window_menu=new QMenu("Window",this);
    window_menu->addAction(ui->linear_chart_dock->toggleViewAction());
    window_menu->addAction(ui->histogram_chart_dock->toggleViewAction());
    window_menu->addAction(ui->phase_plate_dock->toggleViewAction());
    window_menu->addAction(ui->grid_dock->toggleViewAction());
    window_menu->addAction(ui->details_dock->toggleViewAction());
    menuBar()->addMenu(window_menu);
    QMenu *help_menu=new QMenu("Help",this);
    QAction* about_action=help_menu->addAction("About");
    connect(about_action,&QAction::triggered,this, &MainWindow::showAbout);
    menuBar()->addMenu(help_menu);

    default_columns_ << InputOutputVariable("Name","short_name",String)
                     << InputOutputVariable("Timestamp","timestamp",String)
                     << InputOutputVariable("Nom. Defocus","defocus",Float)
                     << InputOutputVariable("Exp. time","exposure_time",Float)
                     << InputOutputVariable("Total Dose","dose",Float)
                     << InputOutputVariable("Pixel size","apix_x",Float)
                     << InputOutputVariable("# Frames","num_frames",Int)
                     << InputOutputVariable("PP","phase_plate_num",Float)
                     << InputOutputVariable("PP position","phase_plate_pos",Float)
                     << InputOutputVariable("PP count","phase_plate_count",Float)
                     << InputOutputVariable("Grid square","grid_square",Float)
                     << InputOutputVariable("Foil hole","grid_square_pos",Float)
                     << InputOutputVariable("Foil hole position","grid_square_hole_pos",Float);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    Settings settings;
    ui->avg_source_dir->setText(settings.value("avg_source_dir").toString());
    ui->stack_source_dir->setText(settings.value("stack_source_dir").toString());
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
    Settings *settings=new Settings;
    settings->beginGroup("DefaultColumns");
    Q_FOREACH(InputOutputVariable column, default_columns_){
        if(settings->value(column.label,true).toBool()){
            model_->addColumn(column);
        }
    }
    settings->endGroup();
    settings->beginGroup("Tasks");
    updateTaskWidget_(settings,NULL,NULL);
    settings->endGroup();
    delete settings;
}

void MainWindow::updateTaskWidget_(Settings *settings, QFormLayout *parent_input_layout,QFormLayout *parent_output_layout)
{
    QList<QColor> colors;
    //colors << QColor(255,240,240)<< QColor(240,255,240)<< QColor(240,255,255)<< QColor(240,240,255)<< QColor(255,255,240)<< QColor(255,240,255);
    colors << QColor(155,140,140)<< QColor(140,155,140)<< QColor(140,155,155)<< QColor(140,140,155)<< QColor(155,155,140)<< QColor(155,140,155);
    static int color_idx=5;
    foreach(QString child_name, settings->childGroups()){
        QFormLayout *input_layout;
        QFormLayout *output_layout;
        settings->beginGroup(child_name);
        bool group_with_parent=settings->value("group_with_parent")==true &&  parent_input_layout && parent_output_layout;
        if (group_with_parent){
            input_layout=parent_input_layout;
            output_layout=parent_output_layout;
        }else{
            input_layout = new QFormLayout;
            output_layout = new QFormLayout;
        }
        QList<QVariant> variant_list=settings->value("input_variables").toList();
        Settings script_input_settings;
        foreach(QVariant v,variant_list){
            InputOutputVariable iov(v);
            QWidget* local_widget;
            QString settings_key="ScriptInput/"+child_name+"/"+iov.label;
            if(iov.type==Image || iov.type==String){
                QLineEdit *le_widget=new QLineEdit();
                if(!script_input_settings.contains(settings_key)){
                    script_input_settings.setValue(settings_key,"");
                }
                local_widget=le_widget;
                connect(local_widget,SIGNAL(textChanged(QString)),this,SLOT(inputDataChanged()));
                le_widget->setText(script_input_settings.value(settings_key).toString());
                local_widget=le_widget;
            }else if(iov.type==Int){
                if(!script_input_settings.contains(settings_key)){
                    script_input_settings.setValue(settings_key,0);
                }
                QSpinBox *sp_widget=new QSpinBox();
                sp_widget->setMaximum(9999999);
                sp_widget->setMinimum(-9999999);
                local_widget=sp_widget;
                connect(local_widget,SIGNAL(valueChanged(int)),this,SLOT(inputDataChanged()));
                sp_widget->setValue(script_input_settings.value(settings_key).toInt());
            }else if(iov.type==Float){
                if(!script_input_settings.contains(settings_key)){
                    script_input_settings.setValue(settings_key,0.0);
                }
                QDoubleSpinBox *sp_widget=new QDoubleSpinBox();
                sp_widget->setMaximum(9999999);
                sp_widget->setMinimum(-9999999);
                local_widget=sp_widget;
                connect(local_widget,SIGNAL(valueChanged(double)),this,SLOT(inputDataChanged()));
                sp_widget->setValue(script_input_settings.value(settings_key).toFloat());
            }else{
                continue;
            }
            local_widget->setStyleSheet(" * {background-color: rgb(136, 138, 133)}");
            input_layout->addRow(iov.key,local_widget);
            local_widget->setProperty("type",iov.type);
            local_widget->setProperty("label",iov.label);
            local_widget->setProperty("task",child_name);

        }
        variant_list=settings->value("output_variables").toList();
        bool color_set=false;
        foreach(QVariant v,variant_list){
            InputOutputVariable iov(v);
            if(iov.in_column){
                if(!color_set){
                    color_set=true;
                    color_idx+=1;
                    color_idx%=colors.size();
                }
                model_->addColumn(iov,colors[color_idx]);
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
                        label->setFixedSize(512,512);
                        QPicture p;
                        label->setPicture(p);
                        output_layout->addRow(label);
                        break;
                }
            }
        }
        updateTaskWidget_(settings,input_layout,output_layout);
        if ( ! group_with_parent){
            QWidget* widget=new QWidget();
            QScrollArea* scroll_area=new QScrollArea();
            QVBoxLayout *layout = new QVBoxLayout;
            QGroupBox *input_group=new QGroupBox("Input");
            layout->addWidget(input_group);
            input_group->setLayout(input_layout);
            QGroupBox *output_group=new QGroupBox("Output");
            layout->addWidget(output_group);
            output_group->setLayout(output_layout);
            layout->addStretch(1);
            widget->setLayout(layout);
            scroll_area->setWidget(widget);
            ui->image_data->insertTab(0,scroll_area,child_name);
        }
        settings->endGroup();
    }
}

void MainWindow::exportChart_(const QString &name, QGraphicsView *view) const
{
    QPrinter printer;
    printer.setOrientation(QPrinter::Landscape);
    QPrintDialog dialog(&printer);
    dialog.setWindowTitle("Print "+name);
    if (dialog.exec() == QDialog::Accepted){
        QPainter painter;
        painter.begin(&printer);
        view->render(&painter, printer.pageRect());
        painter.end();
    }
}

void MainWindow::onAvgSourceDirTextChanged(const QString &dir)
{
    Settings settings;
    settings.setValue("avg_source_dir",ui->avg_source_dir->text());
    settings.saveToFile(".cryoflare.ini", QStringList(), QStringList() << "avg_source_dir");
}

void MainWindow::onStackSourceDirTextChanged(const QString &dir)
{
    Settings settings;
    settings.setValue("stack_source_dir",ui->stack_source_dir->text());
    settings.saveToFile(".cryoflare.ini", QStringList(), QStringList() << "stack_source_dir");
}

void MainWindow::updateDetailsfromModel(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    int current_row=sort_proxy_->mapToSource(ui->image_list->currentIndex()).row();
    chart_update_timer_.start();
    if(topLeft.row()<=current_row && bottomRight.row()>=current_row){
        updateDetails();
    }
}

void MainWindow::updateDetailsfromView(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    static int previous_row=-1,previous_column=-1;
    int current_row=sort_proxy_->mapToSource(ui->image_list->currentIndex()).row();
    int current_column=sort_proxy_->mapToSource(ui->image_list->currentIndex()).column();
    if(current_column!=previous_column){
        previous_column=current_column;
        chart_update_timer_.start();
    }
    if(previous_row!=current_row){
        previous_row=current_row;
        updateDetails();
    }
}

void MainWindow::onSettings()
{
    SettingsDialog settings_dialog(default_columns_,this);
    if (QDialog::Accepted==settings_dialog.exec()){
        settings_dialog.saveSettings();
        Settings settings;
        settings.saveToFile(".cryoflare.ini");
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
       Settings settings;
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
        settings.saveToFile(".cryoflare.ini", QStringList(), QStringList() << "ScriptInput/"+task.toString()+"/"+label.toString());
    }
}

void MainWindow::onExport()
{
    QString export_path;
    Settings settings;
    bool ask_destination=settings.value("ask_destination").toBool();
    if(ask_destination){
        export_path=QFileDialog::getExistingDirectory(0, "Export folder","",  QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);
    }
    if( (! export_path.isEmpty()) || (! ask_destination)){
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

void MainWindow::updateDetails()
{
    QModelIndex idx=sort_proxy_->mapToSource(ui->image_list->currentIndex());
    if(!idx.isValid()){
        return;
    }
    int row=idx.row();
    DataPtr data=model_->image(row);
    int i=ui->image_data->currentIndex();
    QScrollArea *scroll_area=qobject_cast<QScrollArea*>(ui->image_data->widget(i));
    if(!scroll_area){
        qDebug() << "found no scroll area at tab: " << i;
        return;
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
                QPicture p;
                if(QFileInfo(path).exists()){
                    QImageReader reader(path);
                    QSize image_size=reader.size();
                    float scalefactor=std::min(512.0/image_size.width(),512.0/image_size.height());
                    int reduced_x=static_cast<int>(scalefactor*image_size.width());
                    int reduced_y=static_cast<int>(scalefactor*image_size.height());
                    reader.setScaledSize(QSize(reduced_x,reduced_y));
                    QImage image=reader.read();
                    QPainter painter(&p);
                    painter.drawImage(QRect(QPoint((512-reduced_x)/2,(512-reduced_x/2)),QSize(reduced_x,reduced_y)), image, QRect(QPoint(0,0),QSize(reduced_x,reduced_y)));
                }
                qlabel->setPicture(p);
            }else if(static_cast<VariableType>(type.toInt())==String || static_cast<VariableType>(type.toInt())==Float || static_cast<VariableType>(type.toInt())==Int){
                qlabel->setText(data->value(label.toString()));
            }
        }
    }
}

void MainWindow::updateChart()
{
    QElapsedTimer timer;
    timer.start();
    int column=sort_proxy_->mapToSource(ui->image_list->currentIndex()).column();
    if(column<0 || column>= model_->columnCount(QModelIndex())){
        return;
    }
    QList<QPointF> datalist;
    QList<float> histo_datalist;
    for(int i=0;i<model_->rowCount();++i){
        QVariant val=model_->data(model_->index(i,column),ImageTableModel::SortRole);
        DataPtr data=model_->image(i);
        QString export_val=data->value("export","true");
        bool linear_export_flag=export_val.compare("true", Qt::CaseInsensitive) == 0 || export_val==QString("1") || ui->filter->isChecked()==false;
        bool chart_export_flag=export_val.compare("true", Qt::CaseInsensitive) == 0 || export_val==QString("1") || ui->filter->isChecked()==false;
        if(val.canConvert<float>() && val.toString()!=QString("")){
            float fval=val.toFloat();
            if(linear_export_flag){
                datalist.append(QPointF(i,fval));
            }
            if(chart_export_flag){
                histo_datalist.append(fval);
            }
        }
    }
    Settings settings;
    int histogram_bins=settings.value("histogram_bins",256).toInt();
    auto chart_min_max = std::minmax_element(histo_datalist.begin(), histo_datalist.end());
    histogram_min_=*chart_min_max.first;
    float histogram_max=*chart_min_max.second;
    float last_idx=0;
    QList<QPointF> series;
    QList<QList<QPointF> > line_list;
    for(int i=0;i<datalist.size();++i){
        if(datalist[i].x()>last_idx+1 && series.count()>0){
            line_list.append(series);
            series = QList<QPointF>();
        }
        series << datalist[i];
        last_idx=datalist[i].x();
    }
    if(series.count()>0){
        line_list.append(series);
    }

    QVector<float> buckets(histogram_bins);
    if(histogram_max>histogram_min_){
        histogram_bucket_size_=(histogram_max-histogram_min_)/histogram_bins;
    }else{
        histogram_bucket_size_=0.01;
    }
    while (!histo_datalist.isEmpty()){
        float datapoint=histo_datalist.takeFirst();
        int bucket_id=std::max(0,std::min(histogram_bins-1,static_cast<int>(floor((datapoint-histogram_min_)/histogram_bucket_size_))));
        buckets[bucket_id]+=1.0;
    }
    histogram_=buckets;
    QList<QPointF> histogram;
    float half_gap=0.05;
    for(int i=0;i<histogram_bins;++i){
        histogram << QPointF(histogram_min_+(i+half_gap)*histogram_bucket_size_,0) << QPointF(histogram_min_+(i+half_gap)*histogram_bucket_size_,buckets[i]) << QPointF(histogram_min_+(i+1.0-half_gap)*histogram_bucket_size_,buckets[i]) << QPointF(histogram_min_+(i+1.0-half_gap)*histogram_bucket_size_,0);
    }

    QColor color(23,159,223);
    QtCharts::QChart *chart = ui->chart->chart();
    chart->removeAllSeries();
    foreach( QList<QPointF> line, line_list){
        QtCharts::QLineSeries *series = new QtCharts::QLineSeries();
        series->setColor(color);
        series->setPointsVisible();
        series->append(line);
        chart->addSeries(series);
        connect(series,&QtCharts::QLineSeries::hovered,this,&MainWindow::displayLinearChartDetails);
    }
    chart->legend()->hide();
    chart->createDefaultAxes();
    chart->setTitle(model_->headerData(column,Qt::Horizontal,Qt::DisplayRole).toString());
    chart = ui->histogram->chart();
    chart->removeAllSeries();
    QtCharts::QLineSeries *line_series = new QtCharts::QLineSeries();
    line_series->append(histogram);
    line_series->setColor(color);
    QtCharts::QAreaSeries *aseries = new QtCharts::QAreaSeries(line_series);
    aseries->setBrush(QBrush(color));
    connect(aseries,&QtCharts::QAreaSeries::hovered,this,&MainWindow::displayHistogramChartDetails);
    chart->addSeries(aseries);
    chart->legend()->hide();
    chart->createDefaultAxes();
    chart->setTitle(model_->headerData(column,Qt::Horizontal,Qt::DisplayRole).toString());
    updatePhasePlateChart();
    updateGridSquareChart();
}

void MainWindow::updatePhasePlateChart()
{
    int column=sort_proxy_->mapToSource(ui->image_list->currentIndex()).column();
    if(column<0 || column>= model_->columnCount(QModelIndex())){
        return;
    }

    QVector<QVector<float> > data_vectors;
    for(int i=0;i<model_->rowCount();++i){
        QVariant val=model_->data(model_->index(i,column),ImageTableModel::SortRole);
        DataPtr data=model_->image(i);
        QString export_val=data->value("export","true");
        bool phase_plate_export_flag=export_val.compare("true", Qt::CaseInsensitive) == 0 || export_val==QString("1") || ui->filter->isChecked()==false;
        if(val.canConvert<float>() && val.toString()!=QString("")){
            float fval=val.toFloat();
            int phase_plate_num=std::max(1,std::min(6,data->value("phase_plate_num").toInt()));
            int phase_plate_pos_num=data->value("phase_plate_pos").toInt();
            if(phase_plate_export_flag && (phase_plate_level_==0 || phase_plate_num==current_phase_plate_)){
                int index=(phase_plate_level_==0 ? phase_plate_num -1: phase_plate_pos_num);
                while(index>=data_vectors.size()){
                    data_vectors.append(QVector<float>());
                }
                data_vectors[index].append(fval);
            }
        }
    }
    QHash<int,float> data_vectors_avg;
    for(int i=0;i<data_vectors.size();++i){
        if(data_vectors[i].size()>0){
            int n = 0;
            double mean = 0.0;
            for (int j=0;j<data_vectors[i].size();++j) {
                float delta = data_vectors[i][j] - mean;
                mean += delta/++n;
            }
            data_vectors_avg[i]=mean;
        }
    }
    if(phase_plate_level_==0){
        phase_plate_chart_->setValues(data_vectors_avg);
    }else{
        phase_plate_position_chart_->setValues(data_vectors_avg);
    }

}

void MainWindow::updateGridSquareChart()
{
    int column=sort_proxy_->mapToSource(ui->image_list->currentIndex()).column();
    if(column<0 || column>= model_->columnCount(QModelIndex())){
        return;
    }
    QHash<int,QPointF> positions;
    QHash<int,QVector<float> > data_vectors;
    for(int i=0;i<model_->rowCount();++i){
        QVariant val=model_->data(model_->index(i,column),ImageTableModel::SortRole);
        DataPtr data=model_->image(i);
        QString export_val=data->value("export","true");
        bool export_flag=export_val.compare("true", Qt::CaseInsensitive) == 0 || export_val==QString("1") || ui->filter->isChecked()==false;
        if(val.canConvert<float>() && val.toString()!=QString("")){
            float fval=val.toFloat();
            int square_id=data->value("square_id").toInt();
            if(chart_current_square_==-1){
                if(export_flag){
                    if(!data_vectors.contains(square_id)){
                        data_vectors[square_id]=QVector<float>();
                    }
                    data_vectors[square_id].append(fval);
                    positions[square_id]=QPointF(2e5*data->value("square_X").toFloat(),2e5*data->value("square_Y").toFloat());
                }
            }else{
                int hole_id=data->value("hole_id").toInt();
                if(export_flag &&   square_id==chart_current_square_){
                    if(!data_vectors.contains(hole_id)){
                        data_vectors[hole_id]=QVector<float>();
                    }
                    data_vectors[hole_id].append(fval);
                    positions[hole_id]=QPointF(10e6*data->value("X").toFloat(),10e6*data->value("Y").toFloat());
                }
            }
        }
    }
    QHash<int,float> averages;
    for(QHash<int,QPointF>::const_iterator i = positions.constBegin();i != positions.constEnd();++i) {
        int n = 0;
        double avg = 0.0;
        for (int j=0;j<data_vectors[i.key()].size();++j) {
            float delta = data_vectors[i.key()][j] - avg;
            avg += delta/++n;
        }
        averages[i.key()]=avg;
    }
    PositionChart* chart=dynamic_cast<PositionChart*>(ui->grid_square_chart->scene());
    if(!chart){
        return;
    }
    chart->clear();
    QPainterPath path;
    if(chart_current_square_==-1){
        path.addEllipse(QPointF(0,0),5,5);
        chart->addPositions(path,positions,false);
    }else{
        path.addEllipse(QPointF(0,0),10,10);
        chart->addPositions(path,positions,true);
    }
    chart->setValues(averages);
}

void MainWindow::createProcessIndicator(ProcessWrapper *wrapper, int gpu_id)
{
    ProcessIndicator *indicator=new ProcessIndicator(gpu_id);
    process_indicators_.append(indicator);
    statusBar()->addWidget(indicator);
    connect(wrapper,&ProcessWrapper::started,indicator,&ProcessIndicator::started);
    connect(wrapper,&ProcessWrapper::stopped,indicator,&ProcessIndicator::finished);
}

void MainWindow::deleteProcessIndicators()
{
    while(!process_indicators_.empty()){
        ProcessIndicator *indicator=process_indicators_.takeLast();
        statusBar()->removeWidget(indicator);
        indicator->deleteLater();
    }

}

void MainWindow::displayLinearChartDetails(const QPointF &point, bool state)
{
    if(state){
        //ui->linear_chart_details->setText(QString("Image: %1, Value: %2").arg(static_cast<int>(point.x())).arg(point.y()));
    }else{
        //ui->linear_chart_details->setText("");
    }
}

void MainWindow::displayHistogramChartDetails(const QPointF &point, bool state)
{
    if(state){
        int bucket_id=std::min(histogram_.size()-1,static_cast<int>((point.x()-histogram_min_)/histogram_bucket_size_));
        //ui->histogram_chart_details->setText(QString("Bin: %1-%2, Value: %3").arg(histogram_min_+histogram_bucket_size_*bucket_id).arg(histogram_min_+histogram_bucket_size_*(bucket_id+1)).arg(histogram_[bucket_id]));
    }else{
        //ui->histogram_chart_details->setText("");
    }
}

void MainWindow::exportLinearChart()
{
    exportChart_("Linear Chart", ui->chart);
}

void MainWindow::exportHistogramChart()
{
    exportChart_("Histogram Chart", ui->histogram);
}

void MainWindow::exportPhasePlateChart()
{
    exportChart_("Phase Plate Chart", ui->phase_plate);
}

void MainWindow::exportGridSquareChart()
{
    exportChart_("Grid Square Chart", ui->grid_square_chart);
}

void MainWindow::selectFromLinearChart(float start, float end, bool invert)
{
    ui->select->setChecked(false);
    int istart=std::max(0,static_cast<int>(start));
    int iend=std::min(model_->rowCount()-1,static_cast<int>(end));
    if(invert){
        for(int i=istart;i<=iend;++i){
            DataPtr data=model_->image(i);
            data->insert("export","0");
            onDataChanged(data);
        }
    }else{
        for(int i=0;i<istart;++i){
            DataPtr data=model_->image(i);
            data->insert("export","0");
            onDataChanged(data);
        }
        for(int i=iend+1;i<model_->rowCount();++i){
            DataPtr data=model_->image(i);
            data->insert("export","0");
            onDataChanged(data);
        }
    }
}

void MainWindow::selectFromHistogramChart(float start, float end, bool invert)
{
    ui->select->setChecked(false);
    int column=sort_proxy_->mapToSource(ui->image_list->currentIndex()).column();
    if(invert){
        for(int i=0;i<model_->rowCount();++i){
            QVariant val=model_->data(model_->index(i,column),ImageTableModel::SortRole);
            DataPtr data=model_->image(i);
            if(val.canConvert<float>() && val.toString()!=QString("")){
                float fval=val.toFloat();
                if(fval>=start && fval<=end){
                    data->insert("export","0");
                    onDataChanged(data);
                }
            }
        }
    }else{
        for(int i=0;i<model_->rowCount();++i){
            QVariant val=model_->data(model_->index(i,column),ImageTableModel::SortRole);
            DataPtr data=model_->image(i);
            if(val.canConvert<float>() && val.toString()!=QString("")){
                float fval=val.toFloat();
                if(fval<start || fval>end){
                    data->insert("export","0");
                    onDataChanged(data);
                }
            }
        }
    }
}

void MainWindow::onStartStopButton(bool start)
{
    if(start){
        model_->clearData();
    }
    emit startStop(start);
}

void MainWindow::showAbout()
{
    AboutDialog dialog;
    dialog.exec();
}

void MainWindow::phasePlateSelectionChanged()
{
    if(ui->select->isChecked()){
        return;
    }
    if(phase_plate_level_==0){
        QList<QGraphicsItem *> items=phase_plate_chart_->selectedItems();
        if(items.empty()){
            return;
        }
        current_phase_plate_=items[0]->toolTip().toInt();
        ui->phase_plate->setScene(phase_plate_position_chart_);
        ++phase_plate_level_;
        updatePhasePlateChart();
        phase_plate_chart_->clearSelection();
    }else{
        QList<QGraphicsItem *> items=phase_plate_position_chart_->selectedItems();
        if(!items.empty()){
            if(items[0]->toolTip()==QString("back")){
                ui->phase_plate->setScene(phase_plate_chart_);
                --phase_plate_level_;
                current_phase_plate_=-1;
                updatePhasePlateChart();
            }
            phase_plate_position_chart_->clearSelection();
        }
    }
}

void MainWindow::phasePlateSelectionFinished(QRect rubberBandRect, QPointF fromScenePoint, QPointF toScenePoint)
{
    if(rubberBandRect==QRect()){
        bool invert=QGuiApplication::queryKeyboardModifiers()  & Qt::ShiftModifier;
        PositionChart* chart;
        QString tag;
        if(phase_plate_level_==0){
            chart=phase_plate_chart_;
            tag="phase_plate_num";
        }else{
            chart=phase_plate_position_chart_;
            tag="phase_plate_pos";
        }
        QList<QGraphicsItem *> items=chart->selectedItems();
        if(items.empty()){
            return;
        }
        ui->select->setChecked(false);
        chart->clearSelection();
        QSet<QString> ids;
        foreach(QGraphicsItem * item,items){
            ids.insert(item->toolTip());
        }
        for(int i=0;i<model_->rowCount();++i){
            DataPtr data=model_->image(i);
            if(invert){
                if(ids.contains(data->value(tag))){
                    data->insert("export",0);
                    onDataChanged(data);
                }
            }else{
                if(!ids.contains(data->value(tag))){
                    data->insert("export",0);
                    onDataChanged(data);
                }
            }
        }
    }
}

void MainWindow::gridSquareSelectionChanged()
{
    if(ui->select->isChecked()){
        return;
    }
    QList<QGraphicsItem *> items=ui->grid_square_chart->scene()->selectedItems();
    if(items.empty()){
        return;
    }
    if(chart_current_square_==-1){
        chart_current_square_=items[0]->toolTip().toInt();
        updateGridSquareChart();
    }else{
        if(items[0]->toolTip()==QString("back")){
            chart_current_square_=-1;
            updateGridSquareChart();
        }else{
            ui->grid_square_chart->scene()->clearSelection();
        }
    }
}

void MainWindow::gridSquareSelectionFinished(QRect rubberBandRect, QPointF fromScenePoint, QPointF toScenePoint)
{
    if(rubberBandRect==QRect()){
        bool invert=QGuiApplication::queryKeyboardModifiers()  & Qt::ShiftModifier;
        QString tag;
        if(chart_current_square_==-1){
            tag="square_id";
        }else{
            tag="hole_id";
        }
        QList<QGraphicsItem *> items=ui->grid_square_chart->scene()->selectedItems();
        if(items.empty()){
            return;
        }
        ui->select->setChecked(false);
        ui->grid_square_chart->scene()->clearSelection();
        QSet<QString> ids;
        foreach(QGraphicsItem * item,items){
            ids.insert(item->toolTip());
        }
        for(int i=0;i<model_->rowCount();++i){
            DataPtr data=model_->image(i);
            if(invert){
                if(ids.contains(data->value(tag))){
                    data->insert("export",0);
                    onDataChanged(data);
                }
            }else{
                if(!ids.contains(data->value(tag))){
                    data->insert("export",0);
                    onDataChanged(data);
                }
            }
        }
    }
}


void MainWindow::displayScatterPlot()
{
    ScatterPlotDialog dialog(model_);
    dialog.exec();
}

void MainWindow::enableSelection(bool selecting)
{
    ui->chart->enableSelection(selecting);
    ui->histogram->enableSelection(selecting);
    ui->phase_plate->enableSelection(selecting);
    ui->grid_square_chart->enableSelection(selecting);
}




