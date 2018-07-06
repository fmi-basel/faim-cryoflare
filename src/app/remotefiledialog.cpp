#include "remotefiledialog.h"
#include "ui_remotefiledialog.h"

RemoteFileDialog::RemoteFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RemoteFileDialog),
    model_(new QSsh::SftpFileSystemMode(this))
{
    ui->setupUi(this);
}

QString RemoteFileDialog::path()
{
    return model_->data(ui->tree->currentIndex()).toString();
}

QString RemoteFileDialog::getRemotePath()
{
    RemoteFileDialog dialog;
    if(dialog.exex()==QDialog::Accepted){
        return dialog.path();
    }
    return QString();
}

RemoteFileDialog::~RemoteFileDialog()
{
    delete ui;
}
