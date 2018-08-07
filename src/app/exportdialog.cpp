#include "exportdialog.h"
#include "ui_exportdialog.h"
#include "remotefiledialog.h"

ExportDialog::ExportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportDialog)
{
    ui->setupUi(this);
    ui->data_path->setPathType(PathEdit::ExistingDirectory);
    ui->raw_data_path->setPathType(PathEdit::ExistingDirectory);
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

QUrl ExportDialog::destinationPath() const
{
    return ui->data_path->remotePath();
}

QUrl ExportDialog::rawDestinationPath() const
{
    if(ui->separate_raw_path->isChecked()){
        return ui->raw_data_path->remotePath();
    }else{
        return destinationPath();
    }
}

void ExportDialog::setDestinationPath(const QUrl &url)
{
    ui->data_path->setRemotePath(url);
}

void ExportDialog::setRawDestinationPath(const QUrl &url)
{
    ui->raw_data_path->setRemotePath(url);
}

bool ExportDialog::separateRawPath() const
{
    return ui->separate_raw_path->isChecked();
}

void ExportDialog::setSeparateRawPath(bool f)
{
    ui->separate_raw_path->setChecked(f);
}

