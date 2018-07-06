#include "remotefiledialog.h"
#include "ui_remotefiledialog.h"
#include "../external/qssh/sftpfilesystemmodel.h"

RemoteFileDialog::RemoteFileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RemoteFileDialog),
    model_(new QSsh::SftpFileSystemModel(this)),
    ssh_params_()
{
    ui->setupUi(this);
}

QString RemoteFileDialog::path()
{
    return model_->data(ui->tree->currentIndex()).toString();
}

QSsh::SshConnectionParameters RemoteFileDialog::connectionParameters()
{
    return ssh_params_;
}

QPair<QSsh::SshConnectionParameters, QString> RemoteFileDialog::getRemotePath()
{
    RemoteFileDialog dialog;
    if(dialog.exec()==QDialog::Accepted && dialog.path()!=QString()){
        return QPair<QSsh::SshConnectionParameters, QString>(dialog.connectionParameters(),dialog.path());
    }
    return QPair<QSsh::SshConnectionParameters, QString>();
}

RemoteFileDialog::~RemoteFileDialog()
{
    delete ui;
}

void RemoteFileDialog::connectToHost()
{
    ssh_params_.host=ui->host->text();
    ssh_params_.userName=ui->user->text();
    ssh_params_.password=ui->password->text();
    model_->setSshConnection(ssh_params_);
}
