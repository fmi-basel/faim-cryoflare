#include "gridsquareform.h"
#include "ui_gridsquareform.h"

GridsquareForm::GridsquareForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GridsquareForm)
{
    ui->setupUi(this);
}

GridsquareForm::~GridsquareForm()
{
    delete ui;
}
