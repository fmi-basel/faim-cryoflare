//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2020 by the CryoFLARE Authors
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
#include "chartview.h"
#include "micrographsform.h"
#include "pathedit.h"
#include "settings.h"
#include "ui_micrographsform.h"

#include <QAreaSeries>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QGraphicsLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QLineSeries>
#include <QMenu>
#include <QPicture>
#include <QImage>
#include <QSpinBox>
#include <QCheckBox>
#include <QMenuBar>
MicrographsForm::MicrographsForm(QMainWindow *parent) :
    QWidget(parent),
    ui(new Ui::MicrographsForm),
    meta_data_store_(),
    processor_(),
    model_(),
    sort_proxy_(new ImageTableSortFilterProxyModel(this)),
    summary_model_(),
    chart_update_timer_(),
    histogram_min_(0),
    histogram_bucket_size_(1),
    histogram_(),
    phase_plate_chart_(new PositionChart(this)),
    phase_plate_position_chart_(new PositionChart(this)),
    phase_plate_level_(0),
    current_phase_plate_(),
    micrograph_menu_(new QMenu("Micrographs",this)),
    chart_menu_(new QMenu("Charts",this))

{
    ui->setupUi(this);
    ui->chart_splitter->setSizes({100,100,100,100});
    ui->linear_chart->chart()->setTheme(QtCharts::QChart::ChartThemeBlueCerulean);
    ui->linear_chart->chart()->layout()->setContentsMargins(0,0,0,0);
    ui->histogram->chart()->setTheme(QtCharts::QChart::ChartThemeBlueCerulean);
    ui->histogram->chart()->layout()->setContentsMargins(0,0,0,0);
    ui->image_list_summary->setSibling(ui->image_list);
    ui->image_list_summary->setStyleSheet("QHeaderView::section { padding-left: 1 px}");
    ui->image_list->horizontalHeader()->setStyleSheet("QHeaderView::section { padding-left:  8 px}");
    ui->linear_chart->setRenderHints(QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform|QPainter::Antialiasing);
    ui->linear_chart->setOptimizationFlag(QGraphicsView::DontSavePainterState);
    ui->linear_chart->setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing);
    ui->histogram->setRenderHints(QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform|QPainter::Antialiasing);
    ui->histogram->setOptimizationFlag(QGraphicsView::DontSavePainterState);
    ui->histogram->setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing);
    connect(ui->linear_chart, &LinearChartView::indexClicked,ui->image_list , &ImageTableView::jumpToMicrograph);
    
    micrograph_menu_->addAction(ui->image_list->selectAllAction());
    micrograph_menu_->addAction(ui->image_list->deselectAllAction());
    micrograph_menu_->addAction(ui->image_list->selectAboveAction());
    micrograph_menu_->addAction(ui->image_list->deselectAboveAction());
    micrograph_menu_->addAction(ui->image_list->selectBelowAction());
    micrograph_menu_->addAction(ui->image_list->deselectBelowAction());
    micrograph_menu_->addAction(ui->image_list->invertSelectionAction());
    micrograph_menu_->addSeparator();
    QAction* reprocess_all=new QAction("Reprocess all images",this);
    QAction* reprocess_selected=new QAction("Reprocess selected images",this);
    QAction* reprocess_current=new QAction("Reprocess current image",this);
    micrograph_menu_->addAction(reprocess_all);
    micrograph_menu_->addAction(reprocess_selected);
    micrograph_menu_->addAction(reprocess_current);

    connect(reprocess_all,&QAction::triggered,this, &MicrographsForm::reprocessAll);
    connect(reprocess_selected,&QAction::triggered,this, &MicrographsForm::reprocessSelected);
    connect(reprocess_current,&QAction::triggered,this, &MicrographsForm::reprocessCurrent);

    QAction* hide_action=new QAction("Linear Chart");
    hide_action->setCheckable(true);
    hide_action->setChecked(true);
    connect(hide_action,&QAction::triggered,ui->linear_chart,&ChartView::setVisible);
    chart_menu_->addAction(hide_action);
    hide_action=new QAction("Histogram Chart");
    hide_action->setCheckable(true);
    hide_action->setChecked(true);
    connect(hide_action,&QAction::triggered,ui->histogram,&ChartView::setVisible);
    chart_menu_->addAction(hide_action);
    hide_action=new QAction("Phase Plate Chart");
    hide_action->setCheckable(true);
    hide_action->setChecked(true);
    connect(hide_action,&QAction::triggered,ui->phase_plate,&PositionChartView::setVisible);
    chart_menu_->addAction(hide_action);
}

void MicrographsForm::init(QMainWindow *parent, MetaDataStore* store, MicrographProcessor *processor, TaskConfiguration *task_config)
{
    meta_data_store_=store;
    processor_=processor;
    model_=new ImageTableModel(meta_data_store_,task_config, this);
    sort_proxy_->setSourceModel(model_);
    sort_proxy_->setSortRole(ImageTableModel::SortRole);
    ui->image_list->setModel(sort_proxy_);
    summary_model_=new TableSummaryModel(model_,this);
    ui->image_list_summary->setModel(summary_model_);
    chart_update_timer_.setSingleShot(true);
    ui->linear_chart->setModel(model_);
    ui->histogram->setModel(model_);

    parent->menuBar()->addMenu(micrograph_menu_);
    parent->menuBar()->addMenu(chart_menu_);

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
    ui->phase_plate->setScene(phase_plate_chart_);

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
    connect(model_,&ImageTableModel::dataChanged,this,&MicrographsForm::updateDetailsFromModel);
    connect(ui->image_list->selectionModel(),&QItemSelectionModel::currentChanged,this,&MicrographsForm::updateDetailsfromView);
    connect(task_config,&TaskConfiguration::configurationChanged,this,&MicrographsForm::updateTaskWidgets);
    connect(phase_plate_chart_,&PositionChart::selectionChanged,this,&MicrographsForm::phasePlateSelectionChanged);
    connect(ui->phase_plate,&PositionChartView::rubberBandChanged,this,&MicrographsForm::phasePlateSelectionFinished);
    connect(phase_plate_position_chart_,&PositionChart::selectionChanged,this,&MicrographsForm::phasePlateSelectionChanged);
    connect(ui->image_data,&QTabWidget::currentChanged,this,&MicrographsForm::updateDetailsfromView);
    updateTaskWidgets();
}

MicrographsForm::~MicrographsForm()
{
    delete ui;
}

void MicrographsForm::updateDetailsFromModel(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    int current_row=sort_proxy_->mapToSource(ui->image_list->currentIndex()).row();
    chart_update_timer_.start();
    if(topLeft.row()<=current_row && bottomRight.row()>=current_row){
        updateDetails_();
    }
}

void MicrographsForm::updateDetailsfromView()
{
    static int previous_row=-1,previous_column=-1, previous_tab=-1;
    int current_row=sort_proxy_->mapToSource(ui->image_list->currentIndex()).row();
    int current_column=sort_proxy_->mapToSource(ui->image_list->currentIndex()).column();
    if(current_column!=previous_column){
        previous_column=current_column;
        ui->linear_chart->setActiveColumn(current_column);
        ui->histogram->setActiveColumn(current_column);
        chart_update_timer_.start();
    }
    int current_tab=ui->image_data->currentIndex();
    if(previous_row!=current_row || previous_tab!=current_tab){
        previous_row=current_row;
        previous_tab=current_tab;
        updateDetails_();
    }
}

void MicrographsForm::updateTaskWidgets()
{
    for(int i=ui->image_data->count()-1;i>=0;--i){
        QWidget *widget_ptr=ui->image_data->widget(i);
        ui->image_data->removeTab(i);
        delete widget_ptr;
    }
    Settings *settings=new Settings;
    settings->beginGroup("Tasks");
    updateTaskWidget_(settings,nullptr,nullptr);
    settings->endGroup();
    delete settings;
}

void MicrographsForm::updateDetails_()
{
    QModelIndex idx=sort_proxy_->mapToSource(ui->image_list->currentIndex());
    if(!idx.isValid()){
        return;
    }
    int row=idx.row();
    Data data=model_->image(row);
    int i=ui->image_data->currentIndex();
    QScrollArea *scroll_area=qobject_cast<QScrollArea*>(ui->image_data->widget(i));
    if(!scroll_area){
        //qDebug() << "found no scroll area at tab: " << i;
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
                QString path=data.value(label.toString()).toString();
                if(! path.isEmpty() && QFileInfo::exists(path)){
                    qlabel->setPixmap(QPixmap(path).scaled(QSize(512,512),Qt::KeepAspectRatio,Qt::SmoothTransformation));
                }else{
                    qlabel->setPixmap(QPixmap());
                }
            }else if(static_cast<VariableType>(type.toInt())==Bool || static_cast<VariableType>(type.toInt())==String || static_cast<VariableType>(type.toInt())==Float || static_cast<VariableType>(type.toInt())==Int){
                qlabel->setText(data.value(label.toString()).toString());
            }
        }
    }
}

void MicrographsForm::updateTaskWidget_(Settings *settings, QFormLayout *parent_input_layout, QFormLayout *parent_output_layout)
{
    QList<QColor> colors;
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
            if(iov.type==String){
                QLineEdit *le_widget=new QLineEdit();
                if(!script_input_settings.contains(settings_key)){
                    script_input_settings.setValue(settings_key,"");
                    script_input_settings.saveToFile(CRYOFLARE_INI, QStringList(), QStringList() << settings_key);
                }
                local_widget=le_widget;
                connect(local_widget,SIGNAL(textChanged(QString)),this,SLOT(inputDataChanged()));
                le_widget->setText(script_input_settings.value(settings_key).toString());
                local_widget=le_widget;
                local_widget->setStyleSheet(" * {background-color: rgb(136, 138, 133)}");
            }else if(iov.type==Image){
                PathEdit *le_widget=new PathEdit(PathEdit::OpenFileName);
                if(!script_input_settings.contains(settings_key)){
                    script_input_settings.setValue(settings_key,"");
                    script_input_settings.saveToFile(CRYOFLARE_INI, QStringList(), QStringList() << settings_key);
                }
                connect(le_widget,&PathEdit::pathChanged,this,&MicrographsForm::inputDataChanged);
                le_widget->setPath(script_input_settings.value(settings_key).toString());
                local_widget=le_widget;
                local_widget->setStyleSheet(" * {background-color: rgb(136, 138, 133)}");
            }else if(iov.type==Int){
                if(!script_input_settings.contains(settings_key)){
                    script_input_settings.setValue(settings_key,0);
                    script_input_settings.saveToFile(CRYOFLARE_INI, QStringList(), QStringList() << settings_key);
                }
                QSpinBox *sp_widget=new QSpinBox();
                sp_widget->setMaximum(9999999);
                sp_widget->setMinimum(-9999999);
                local_widget=sp_widget;
                local_widget->setStyleSheet(" * {background-color: rgb(136, 138, 133)}");
                connect(local_widget,SIGNAL(valueChanged(int)),this,SLOT(inputDataChanged()));
                sp_widget->setValue(script_input_settings.value(settings_key).toInt());
            }else if(iov.type==Float){
                if(!script_input_settings.contains(settings_key)){
                    script_input_settings.setValue(settings_key,0.0);
                    script_input_settings.saveToFile(CRYOFLARE_INI, QStringList(), QStringList() << settings_key);
                }
                QDoubleSpinBox *sp_widget=new QDoubleSpinBox();
                sp_widget->setMaximum(9999999);
                sp_widget->setMinimum(-9999999);
                sp_widget->setDecimals(5);
                local_widget=sp_widget;
                local_widget->setStyleSheet(" * {background-color: rgb(136, 138, 133)}");
                connect(local_widget,SIGNAL(valueChanged(double)),this,SLOT(inputDataChanged()));
                sp_widget->setValue(script_input_settings.value(settings_key).toDouble());
            }else if(iov.type==Bool){
                if(!script_input_settings.contains(settings_key)){
                    script_input_settings.setValue(settings_key,false);
                    script_input_settings.saveToFile(CRYOFLARE_INI, QStringList(), QStringList() << settings_key);
                }
                QCheckBox *sp_widget=new QCheckBox();
                local_widget=sp_widget;
                connect(local_widget,SIGNAL(stateChanged(int)),this,SLOT(inputDataChanged()));
                sp_widget->setChecked(script_input_settings.value(settings_key).toBool());
                local_widget->setStyleSheet(" QCheckBox::indicator::unchecked {background-color: rgb(136, 138, 133)} QCheckBox::indicator::checked {background-color: #e6e6e6; border: 3px solid rgb(136, 138, 133);}");
            }else{
                continue;
            }
            input_layout->addRow(iov.key,local_widget);
            local_widget->setProperty("type",iov.type);
            local_widget->setProperty("label",iov.label);
            local_widget->setProperty("task",child_name);

        }
        variant_list=settings->value("output_variables").toList();
        bool color_set=false;
        foreach(QVariant v,variant_list){
            InputOutputVariable iov(v);
            if(!color_set){
                color_set=true;
                color_idx+=1;
                color_idx%=colors.size();
            }
            if(! iov.in_column){
                QLabel *label=new QLabel();
                label->setProperty("type",iov.type);
                label->setProperty("label",iov.label);
                switch(iov.type){
                    case String:
                    case Int:
                    case Float:
                    case Bool:
                        output_layout->addRow(iov.key,label);
                        break;
                    case Image:
                        output_layout->addRow(new QLabel(iov.key));
                        label->setFixedSize(512,512);
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

void MicrographsForm::updatePhasePlateChart()
{
    int column=sort_proxy_->mapToSource(ui->image_list->currentIndex()).column();
    if(column<0 || column>= model_->columnCount(QModelIndex())){
        return;
    }
    QHash<int,QVector<float> > data_vectors;
    for(int i=0;i<model_->rowCount();++i){
        QVariant val=model_->data(model_->index(i,column),ImageTableModel::SortRole);
        Data data=model_->image(i);
        QString export_val=data.value("export").toString("true");
        //bool phase_plate_export_flag=export_val.compare("true", Qt::CaseInsensitive) == 0 || export_val==QString("1") || ui->filter->isChecked()==false;
        bool phase_plate_export_flag=export_val.compare("true", Qt::CaseInsensitive) == 0 || export_val==QString("1") ;
        if(val.canConvert<float>() && val.toString()!=QString("")){
            float fval=val.toFloat();
            int phase_plate_num=std::max(1,std::min(6,data.value("phase_plate_num").toString().toInt()));
            int phase_plate_pos_num=data.value("phase_plate_pos").toString().toInt();
            if(phase_plate_export_flag && (phase_plate_level_==0 || phase_plate_num==current_phase_plate_)){
                int index=(phase_plate_level_==0 ? phase_plate_num : phase_plate_pos_num);
                if(!data_vectors.contains(i)){
                    data_vectors[i]=QVector<float>();
                }
                data_vectors[index].append(fval);
            }
        }
    }
    QHash<int,double> data_vectors_avg;
    for(int i=0;i<data_vectors.size();++i){
        if(data_vectors[i].size()>0){
            int n = 0;
            double mean = 0.0;
            for (int j=0;j<data_vectors[i].size();++j) {
                double delta = data_vectors[i][j] - mean;
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

void MicrographsForm::inputDataChanged()
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
            settings.setValue(label.toString(),qobject_cast<QLineEdit*>(sender_widget)->text());
            break;
        case Image:
            settings.setValue(label.toString(),qobject_cast<PathEdit*>(sender_widget)->path());
            break;
        case Int:
            settings.setValue(label.toString(),qobject_cast<QSpinBox*>(sender_widget)->value());
            break;
        case Float:
            settings.setValue(label.toString(),qobject_cast<QDoubleSpinBox*>(sender_widget)->value());
            break;
        case Bool:
            settings.setValue(label.toString(),qobject_cast<QCheckBox*>(sender_widget)->isChecked());
            break;
        }
        settings.endGroup();
        settings.endGroup();
        settings.saveToFile(CRYOFLARE_INI, QStringList(), QStringList() << "ScriptInput/"+task.toString()+"/"+label.toString());
    }
}

void MicrographsForm::reprocessCurrent()
{
    int current_row=sort_proxy_->mapToSource(ui->image_list->currentIndex()).row();
    processor_->reprocessMicrograph(model_->id(current_row));
}

void MicrographsForm::reprocessSelected()
{
    foreach(QString id,meta_data_store_->selectedMicrographIDs()){
        processor_->reprocessMicrograph(id);
    }
}

void MicrographsForm::reprocessAll()
{
    foreach(QString id,meta_data_store_->micrographIDs()){
        processor_->reprocessMicrograph(id);
    }
}


void MicrographsForm::changeEvent(QEvent *event)
{
    if(event->type()==QEvent::EnabledChange){
        micrograph_menu_->menuAction()->setVisible(isEnabled());
        chart_menu_->menuAction()->setVisible(isEnabled());
    }
    QWidget::changeEvent(event);
}

void MicrographsForm::phasePlateSelectionChanged()
{
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
void MicrographsForm::phasePlateSelectionFinished(QRect rubberBandRect, QPointF /*fromScenePoint*/, QPointF /*toScenePoint*/)
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
        chart->clearSelection();
        QSet<QString> ids;
        foreach(QGraphicsItem * item,items){
            ids.insert(item->toolTip());
        }
        QSet<QString> ids_to_update;
        foreach(QString id, meta_data_store_->selectedMicrographIDs() ){
            Data data=meta_data_store_->micrograph(id);
            if(invert){
                if(ids.contains(data.value(tag).toString())){
                    ids_to_update.insert(id);
                }
            }else{
                if(!ids.contains(data.value(tag).toString())){
                    ids_to_update.insert(id);
                }
            }
        }
        meta_data_store_->setMicrographsExport(ids_to_update,false);
    }
}
