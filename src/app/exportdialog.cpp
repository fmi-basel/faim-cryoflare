#include <QListWidget>
#include <QMessageBox>
#include "exportdialog.h"
#include "ui_exportdialog.h"
#include "remotefiledialog.h"
#include "sshauthenticationdialog.h"

ExportDialog::ExportDialog(const QStringList &raw_keys, const QStringList &output_keys, const QStringList &shared_keys, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportDialog),
    connection_(nullptr)
{
    ui->setupUi(this);
    ui->data_path->setPathType(PathEdit::ExistingDirectory);
    ui->raw_data_path->setPathType(PathEdit::ExistingDirectory);
    ui->data_list->addItems(output_keys);
    ui->data_list->selectAll();
    ui->raw_data_list->addItems(raw_keys);
    ui->raw_data_list->selectAll();
    ui->shared_data_list->addItems(shared_keys);
    ui->shared_data_list->selectAll();
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

SftpUrl ExportDialog::destinationPath() const
{
    return ui->data_path->remotePath();
}

SftpUrl ExportDialog::rawDestinationPath() const
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

bool ExportDialog::duplicateRaw() const
{
    return ui->duplicate_raw->isChecked();
}

void ExportDialog::setDuplicateRaw(bool f)
{
    ui->duplicate_raw->setChecked(f);
}

QStringList ExportDialog::selectedOutputKeys() const
{
    QStringList result;
    foreach(QListWidgetItem* i, ui->data_list->selectedItems()){
        result << i->text();
    }
    return result;
}

QStringList ExportDialog::selectedRawKeys() const
{
    QStringList result;
    foreach(QListWidgetItem* i, ui->raw_data_list->selectedItems()){
        result << i->text();
    }
    return result;
}

QStringList ExportDialog::selectedSharedKeys() const
{
    QStringList result;
    foreach(QListWidgetItem* i, ui->shared_data_list->selectedItems()){
        result << i->text();
    }
    return result;
}

void ExportDialog::verifyDestinations()
{
    connection_=new QSsh::SshConnection(destinationPath().toConnectionParameters(),this);
    connect(connection_,&QSsh::SshConnection::connected,this,&ExportDialog::destinationVerified);
    connect(connection_,&QSsh::SshConnection::error,this,&ExportDialog::destinationVerificationError);
    connection_->connectToHost();
}

void ExportDialog::destinationVerified()
{
    if(separateRawPath()){
        connection_->deleteLater();
        connection_=new QSsh::SshConnection(rawDestinationPath().toConnectionParameters(),this);
        connect(connection_,&QSsh::SshConnection::connected,this,&ExportDialog::accept);
        connect(connection_,&QSsh::SshConnection::error,this,&ExportDialog::rawDestinationVerificationError);
        connection_->connectToHost();
    }else{
        accept();
    }

}

void ExportDialog::destinationVerificationError(QSsh::SshError e)
{
    SftpUrl dest=destinationPath();
    if(e==QSsh::SshError::SshAuthenticationError){
        SshAuthenticationDialog::auth_type auth=SshAuthenticationDialog::getSshAuthentication(QString("Authentication for: %1").arg(dest.toString(QUrl::RemovePassword)));
        if(auth.second!=""){
            dest.setAuthType(auth.first);
            if(auth.first==QSsh::SshConnectionParameters::AuthenticationByPassword){
                dest.setPassword(auth.second);
            }else{
                dest.setKey(auth.second);
            }
            setDestinationPath(dest);
            verifyDestinations();
        }
    }else{
        QMessageBox message_box;
        message_box.setIcon(QMessageBox::Critical);
        message_box.setText(QString("Error connecting to %1: %2").arg(dest.toString(QUrl::RemovePassword)).arg(connection_->errorString()));
        message_box.exec();
    }
}

void ExportDialog::rawDestinationVerificationError(QSsh::SshError e)
{
    SftpUrl raw_dest=rawDestinationPath();
    if(e==QSsh::SshError::SshAuthenticationError){
        SshAuthenticationDialog::auth_type auth=SshAuthenticationDialog::getSshAuthentication(QString("Authentication for: %1").arg(raw_dest.toString(QUrl::RemovePassword)));
        if(auth.second!=""){
            raw_dest.setAuthType(auth.first);
            if(auth.first==QSsh::SshConnectionParameters::AuthenticationByPassword){
                raw_dest.setPassword(auth.second);
            }else{
                raw_dest.setKey(auth.second);
            }
            setRawDestinationPath(raw_dest);
            verifyDestinations();
        }
    }else{
        QMessageBox message_box;
        message_box.setIcon(QMessageBox::Critical);
        message_box.setText(QString("Error connecting to %1: %2").arg(raw_dest.toString(QUrl::RemovePassword)).arg(connection_->errorString()));
        message_box.exec();
    }
}

