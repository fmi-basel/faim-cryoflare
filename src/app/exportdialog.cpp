#include "exportdialog.h"
#include "ui_exportdialog.h"
#include "remotefiledialog.h"

ExportDialog::ExportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportDialog),
    progress_dialog_(new QProgressDialog("Connecting...", "Abort", 0, 0, this))
{
    ui->setupUi(this);
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

void ExportDialog::browseRemoteData()
{
    QPair<QSsh::SshConnectionParameters,QString> pair=RemoteFileDialog::getRemotePath();
    if(pair.second!=QString()){
        ui->data_path->setPath(pair.second);
    }
}

void ExportDialog::browseRemoteRawData()
{
    QPair<QSsh::SshConnectionParameters,QString> pair=RemoteFileDialog::getRemotePath();
    if(pair.second!=QString()){
        ui->data_path->setPath(pair.second);
    }
}
