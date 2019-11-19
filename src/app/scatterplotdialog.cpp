#include <QChart>
#include <QGraphicsLayout>
#include <QtCharts/QScatterSeries>
#include "scatterplotdialog.h"
#include "ui_scatterplotdialog.h"

ScatterPlotDialog::ScatterPlotDialog(MetaDataStore * store, QList<InputOutputVariable> result_labels, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScatterPlotDialog),
    store_(store),
    result_labels_(result_labels)
{
    ui->setupUi(this);
    ui->chart->chart()->setTheme(QtCharts::QChart::ChartThemeBlueCerulean);
    ui->chart->setRenderHint(QPainter::Antialiasing);
    ui->chart->chart()->layout()->setContentsMargins(0,0,0,0);
    QStringList columns;
    foreach(InputOutputVariable v, result_labels_){
        columns << v.label;
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
    QStringList ids;
    if(ui->filter_chart->isChecked()){
        ids=store_->selectedMicrographIDs();
    }else{
        ids=store_->micrographIDs();
    }
    foreach(QString id, ids){
        Data data=store_->micrograph(id);
        QVariant val_x=data.value(result_labels_.value(ui->list_x->currentRow()).key);
        QVariant val_y=data.value(result_labels_.value(ui->list_y->currentRow()).key);
        if(val_x.canConvert<float>() && val_x.toString()!=QString("") && val_y.canConvert<float>() && val_y.toString()!=QString("") ){
            float fval_x=val_x.toFloat();
            float fval_y=val_y.toFloat();
            series->append(fval_x,fval_y);
        }
    }
    ui->chart->chart()->removeAllSeries();
    ui->chart->chart()->addSeries(series);
    ui->chart->chart()->createDefaultAxes();
    ui->chart->chart()->axisX(series)->setTitleText(ui->list_x->currentItem()->text());
    ui->chart->chart()->axisY(series)->setTitleText(ui->list_y->currentItem()->text());
    ui->chart->chart()->legend()->hide();

}
