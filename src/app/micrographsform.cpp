#include "micrographsform.h"
#include "ui_micrographsform.h"

MicrographsForm::MicrographsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MicrographsForm)
{
    ui->setupUi(this);
}

MicrographsForm::~MicrographsForm()
{
    delete ui;
}
