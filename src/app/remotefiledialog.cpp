#include "remotefiledialog.h"
#include "ui_remotefiledialog.h"
#include "../external/qssh/sftpfilesystemmodel.h"
#include "../external/qssh/sshconnection.h"

RemoteFileDialog::RemoteFileDialog(const QUrl& remote_path,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RemoteFileDialog),
    model_(new QSsh::SftpFileSystemModel()),
    proxy_(new QSortFilterProxyModel(this)),
    remote_path_(remote_path)
{
    ui->setupUi(this);
    ui->tree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    proxy_->setSourceModel(model_);
    ui->tree->setModel(proxy_);
    proxy_->sort(1);
    connect(model_,&QSsh::SftpFileSystemModel::connectionError,this,&RemoteFileDialog::onConnectionError);
    connect(model_,&QSsh::SftpFileSystemModel::connectionEstablished,this,&RemoteFileDialog::onConnectionEstablished);
    if(remote_path.isValid()){
        ui->host->setText(remote_path.host());
        ui->user->setText(remote_path.userName());
        ui->port->setText(QString("%1").arg(remote_path.port()));
        ui->password->setText(remote_path.password());
        //if(!remote_path.path().isEmpty()){
         // sftp filesystem model doesn't provide a way to get the index for a remote path
         // therefore it is currently not possible to set the current index of the view to the previous path
        //}
    }
    remote_path_.setScheme("sftp");
}

QUrl RemoteFileDialog::remotePath() const
{
    QString path=proxy_->data(ui->tree->currentIndex(),QSsh::SftpFileSystemModel::PathRole).toString();
    if(path.isEmpty()){
        return QUrl();
    }
    QUrl remote_path=remote_path_;
    remote_path.setPath(path);
    return remote_path;
}


QUrl RemoteFileDialog::getRemotePath(const QUrl& path )
{
    RemoteFileDialog dialog(path);
    if(dialog.exec()==QDialog::Accepted){
        QUrl new_path=dialog.remotePath();
        if(new_path.isValid()){
            return new_path;
        }
    }
    return QUrl();
}

RemoteFileDialog::~RemoteFileDialog()
{
    delete ui;
    delete model_;
}

void RemoteFileDialog::connectToHost(bool con)
{
    if(con){
        //connect
        ui->message->setText("Connecting ...");
        QSsh::SshConnectionParameters params;
        remote_path_.setHost(ui->host->text());
        remote_path_.setUserName(ui->user->text());
        remote_path_.setPassword(ui->password->text());
        remote_path_.setPort(ui->port->text().toInt());
        params.host=ui->host->text();
        params.userName=ui->user->text();
        params.password=ui->password->text();
        params.authenticationType=QSsh::SshConnectionParameters::AuthenticationByPassword;
        params.timeout=10;
        params.port=ui->port->text().toInt();
        params.proxyType=QSsh::SshConnectionParameters::NoProxy;
        model_->setSshConnection(params);

    }else{
        //disconnect
    }
}

void RemoteFileDialog::onConnectionEstablished()
{
    ui->message->setText("Connected");
}

void RemoteFileDialog::onConnectionError(const QString &error)
{
    ui->message->setText("Connection Error: "+error);
}
