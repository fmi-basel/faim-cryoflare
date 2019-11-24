#include "task.h"
#include "gridsquareform.h"
#include "ui_gridsquareform.h"
#include <QJsonArray>
#include <algorithm>

struct Normalize {
    qreal min_;
    qreal i_diff_;
    Normalize(qreal min, qreal max):min_(min),i_diff_(1.0/(max-min)){}
    qreal operator() (qreal v){return (v-min_)*i_diff_;}
};

GridsquareForm::GridsquareForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GridsquareForm),
    meta_data_store_(nullptr),
    gridsquare_model_(nullptr),
    task_configuration_(nullptr),
    gradient_({{0.0,QColor(255,0,0,100)},{0.3333,QColor(255,255,0,100)},{0.6666,QColor(0,255,0,100)},{1.0,QColor(0,0,255,100)}})
{
    ui->setupUi(this);

}

GridsquareForm::~GridsquareForm()
{
    delete ui;
}

void GridsquareForm::init(MetaDataStore* meta_data_store, TaskConfiguration* task_config)
{
    meta_data_store_=meta_data_store;
    task_configuration_=task_config;
    gridsquare_model_=new GridsquareTableModel(meta_data_store);
    ui->gridsquare_list->setModel(gridsquare_model_);
    if(gridsquare_model_->columnCount(QModelIndex())>0){
        ui->gridsquare_list->setCurrentIndex(gridsquare_model_->index(0,0));
    }
    connect(task_configuration_, &TaskConfiguration::configurationChanged,this, &GridsquareForm::updateResultLabels);
    connect(ui->result_labels,&QListWidget::currentRowChanged,this,&GridsquareForm::updateMarkers);
}

void GridsquareForm::loadGridsquare()
{
    if(!meta_data_store_){
        return;
    }
    QModelIndex idx=ui->gridsquare_list->currentIndex();
    if(idx.isValid()){
        QString id=gridsquare_model_->data(idx).toString();
        if(meta_data_store_->gridsquare(id).contains("micrograph")){
            QString path=meta_data_store_->gridsquare(id).value("micrograph").toString();
            ui->gridsquare_view->load(path);
            updateMarkers();
        }else{
            ui->gridsquare_view->clear();
        }
    }
}

void GridsquareForm::updateMarkers()
{
    if(!meta_data_store_){
        return;
    }
    ui->gridsquare_view->clearMarkers();
    QModelIndex idx=ui->gridsquare_list->currentIndex();
    QListWidgetItem* current_label=ui->result_labels->currentItem();
    if(idx.isValid() && current_label){
        QString id=gridsquare_model_->data(idx).toString();
        QString result_label=current_label->text();
        Data grid_data=meta_data_store_->gridsquare(id);
        QJsonArray hole_ids=grid_data.value("hole_ids").toArray();
        qDebug()<< result_label <<hole_ids.size();
        if(result_label=="Acquisition state"){
            foreach(QJsonValue fh_id,hole_ids){
                Data fh_data=meta_data_store_->foilhole(fh_id.toString());
                if(fh_data.value("selected").toString()=="true"){
                    QColor color(0,127,0,100);
                    if(fh_data.contains("micrograph_ids")){
                        if(!fh_data.value("micrograph_ids").toArray().empty()){
                            color=QColor(0,0,127,100);
                        }
                    }
                    ui->gridsquare_view->addMarker(QPointF(fh_data.value("x").toString().toDouble(),fh_data.value("y").toString().toDouble()),30,fh_data.value("id").toString(),color);
                }
            }
            QLinearGradient lin_gradient;
            QGradientStops stops;
            stops << QGradientStop(0,QColor(0,127,0,100))<< QGradientStop(1,QColor(0,0,127,100));
            lin_gradient.setStops(stops);
            ui->gridsquare_view->setLegend(lin_gradient,"Selected","Acquired");
        }else {
            QList<QPointF> pos;
            QList<qreal> v;
            QList<QString> ids;
            foreach(QJsonValue fh_id,hole_ids){
                Data fh_data=meta_data_store_->foilhole(fh_id.toString());
                QJsonArray micrograph_ids=fh_data.value("micrograph_ids").toArray();
                int num=0;
                qreal sum=0;
                foreach(QJsonValue micrograph_id,micrograph_ids){
                    Data data=meta_data_store_->micrograph(micrograph_id.toString());
                    if(data.contains(result_label)){
                        double v=data.value(result_label).toString().toDouble();
                        if(! qIsNaN(v)){
                            sum+=v;
                            ++num;
                        }
                    }
                }
                if(num>0){
                    v.append(sum/num);
                    pos.append(QPointF(fh_data.value("x").toString().toDouble(),fh_data.value("y").toString().toDouble()));
                    ids.append(fh_data.value("id").toString());
                }
            }
            const auto [min, max] = std::minmax_element(v.begin(),v.end());
            QLinearGradient lin_gradient;
            QGradientStops stops;
            QMap<qreal,QColor> s=gradient_.stops();
            foreach(qreal v,s.keys()){
                stops << QGradientStop(v,s[v]);
            }
            lin_gradient.setStops(stops);
            ui->gridsquare_view->setLegend(lin_gradient,QString("%1").arg(*min),QString("%1").arg(*max));
            if(*max>*min){
                std::transform(v.begin(),v.end(),v.begin(),Normalize(*min,*max));
            }else{
                std::transform(v.begin(),v.end(),v.begin(),Normalize(*min,*min+1.0));
            }
            QList<QColor> colors=gradient_.createColors(v);
            for(int i=0;i<pos.size();++i){
                ui->gridsquare_view->addMarker(pos[i],30,ids[i],colors[i]);
            }
        }
    }
}

void GridsquareForm::updateResultLabels()
{
    if(!task_configuration_){
        return;
    }
    ui->result_labels->clear();
    ui->result_labels->addItem("Acquisition state");
    foreach(InputOutputVariable v, task_configuration_->resultLabels()){
        if(v.in_column){
            ui->result_labels->addItem(v.label);
        }
    }
    if(ui->result_labels->count()>0){
        ui->result_labels->setCurrentRow(0);
    }
}
