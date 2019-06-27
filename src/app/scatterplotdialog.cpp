#include <QChart>
#include <QGraphicsLayout>
#include <QtCharts/QScatterSeries>
#include "scatterplotdialog.h"
#include "ui_scatterplotdialog.h"

ScatterPlotDialog::ScatterPlotDialog(ImageTableModel * model,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScatterPlotDialog),
    model_(model)
{
    ui->setupUi(this);
    ui->chart->chart()->setTheme(QtCharts::QChart::ChartThemeBlueCerulean);
    ui->chart->setRenderHint(QPainter::Antialiasing);
    ui->chart->chart()->layout()->setContentsMargins(0,0,0,0);
    QStringList columns;
    for(int i=0;i<model_->columnCount(QModelIndex());++i){
        columns << model_->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString();
    }
    ui->list_x->addItems(columns);
    ui->list_x->setCurrentRow(0);
    connect(ui->list_x,&QListWidget::currentRowChanged,this,&ScatterPlotDialog::updateChart);
    ui->list_y->addItems(columns);
    ui->list_y->setCurrentRow(0);
    connect(ui->list_y,&QListWidget::currentRowChanged,this,&ScatterPlotDialog::updateChart);
}

ScatterPlotDialog::~ScatterPlotDialog()
{
    delete ui;
}

void ScatterPlotDialog::updateChart()
{
    QtCharts::QScatterSeries *series = new QtCharts::QScatterSeries();
    series->setMarkerSize(6);
    series->setPen(QPen(QBrush(Qt::white),1));
    series->setColor(QColor(23,159,223));
    for(int i=0;i<model_->rowCount();++i){
        QVariant val_x=model_->data(model_->index(i,ui->list_x->currentRow()),ImageTableModel::SortRole);
        QVariant val_y=model_->data(model_->index(i,ui->list_y->currentRow()),ImageTableModel::SortRole);
        DataPtr data=model_->image(i);
        QString export_val=data->value("export").toString("true");
        bool export_flag=export_val.compare("true", Qt::CaseInsensitive) == 0 || export_val==QString("1") || ui->filter_chart->isChecked()==false;
        if(val_x.canConvert<float>() && val_x.toString()!=QString("") && val_y.canConvert<float>() && val_y.toString()!=QString("") ){
            if(export_flag){
                float fval_x=val_x.toFloat();
                float fval_y=val_y.toFloat();
                series->append(fval_x,fval_y);
            }
        }
    }
    ui->chart->chart()->removeAllSeries();
    ui->chart->chart()->addSeries(series);
    ui->chart->chart()->createDefaultAxes();
    ui->chart->chart()->axisX(series)->setTitleText(ui->list_x->currentItem()->text());
    ui->chart->chart()->axisY(series)->setTitleText(ui->list_y->currentItem()->text());
    ui->chart->chart()->legend()->hide();

}
