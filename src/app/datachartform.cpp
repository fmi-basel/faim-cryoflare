#include "datachartform.h"
#include "ui_datachartform.h"

DataChartForm::DataChartForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataChartForm)
{
    ui->setupUi(this);
}

DataChartForm::~DataChartForm()
{
    delete ui;
}
