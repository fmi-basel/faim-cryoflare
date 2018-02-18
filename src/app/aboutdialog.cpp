#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    ui->version->setText("Version: "+QString(GIT_VERSION));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
