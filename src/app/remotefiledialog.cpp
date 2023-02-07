//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2020 by the CryoFLARE Authors
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3.0 of the License.
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License
// along with CryoFLARE.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------------
#include "remotefiledialog.h"
#include "ui_remotefiledialog.h"
#include "../external/qssh/sftpfilesystemmodel.h"
#include "../external/qssh/sshconnection.h"
#include "sshauthenticationstore.h"
#include "sshauthenticationdialog.h"
#include <QFileInfo>
#include <QDir>

RemoteFileDialog::RemoteFileDialog(const SftpUrl &remote_path, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RemoteFileDialog),
    model_(),
    proxy_(new QSortFilterProxyModel(this)),
    remote_path_(remote_path),
    initial_path_(remote_path_.path().split("/",QString::SkipEmptyParts)),
    initial_idx_(),
    default_keys_(),
    stored_connection_(false)
{
    ui->setupUi(this);
    ui->tree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    initSftpFileSystemModel_();
    if(remote_path.isValid()){
        ui->host->setText(remote_path.host());
        ui->user->setText(remote_path.userName());
        ui->port->setText(QString("%1").arg(remote_path.port()));
    }else{
        ui->port->setText(QString("%1").arg(22));
    }
    remote_path_.setScheme("sftp");
    SshAuthenticationStore store;
    if(store.contains(remotePath().toConnectionParameters())){
        connectToHost(true);
    }
}

SftpUrl RemoteFileDialog::remotePath() const
{
    QModelIndex idx=ui->tree->currentIndex();
    QString path=proxy_->data(proxy_->index( idx.row(),1,idx.parent()),QSsh::SftpFileSystemModel::PathRole).toString();
    if(path.isEmpty()){
        return SftpUrl();
    }
    SftpUrl remote_path=remote_path_;
    remote_path.setPath(path);
    return remote_path;
}


SftpUrl RemoteFileDialog::getRemotePath(const SftpUrl &path )
{
    RemoteFileDialog dialog(path);
    if(dialog.exec()==QDialog::Accepted){
        SftpUrl new_path=dialog.remotePath();
        if(new_path.isValid()){
            return new_path;
        }
    }
    return SftpUrl();
}

RemoteFileDialog::~RemoteFileDialog()
{
    delete ui;
    delete model_;
}

void RemoteFileDialog::connectToHost(bool con)
{
    if(con){
        ui->connect->setText("Disconnect");
        default_keys_.clear();
        QStringList default_paths;
        //default_paths << QDir::homePath()+"/.ssh/id_ecdsa";
        default_paths << QDir::homePath()+ "/.ssh/id_ed25519";
        default_paths << QDir::homePath()+"/.ssh/id_dsa";
        default_paths << QDir::homePath()+"/.ssh/id_rsa";
        foreach(QString path,default_paths){
            if(QFileInfo::exists(path)){
                default_keys_ << path;
            }
        }
        tryNextAuth_();
    }else{
        disconnect_();
    }
}

void RemoteFileDialog::onConnectionEstablished()
{
    SshAuthenticationStore store;
    store.store(remote_path_.toConnectionParameters());
    ui->message->setText("Connected");
}

void RemoteFileDialog::onConnectionError(const QString &error)
{
    initSftpFileSystemModel_();
    if(error==tr("Server rejected password.") || error==tr("Server rejected key.")){
        qDebug() << "connection error: " <<error;
        tryNextAuth_();
    }else{
        ui->message->setText("Connection Error: "+error);
        disconnect_();
    }
}

void RemoteFileDialog::modelReady_()
{
    if(!initial_path_.empty()){
        QString name=model_->data(initial_idx_,Qt::DisplayRole).toString();
        if(!initial_idx_.isValid()){
            initial_idx_=model_->index(0,0);
        }
        if(model_->isFetching(initial_idx_)){
            QString folder=initial_path_.takeFirst();
            bool found=false;
            for(int i=0;i<model_->rowCount(initial_idx_);++i){
                QString name=model_->data(model_->index(i,1,initial_idx_),Qt::DisplayRole).toString();
                if(name==folder){
                    QModelIndex new_idx=model_->index(i,0,initial_idx_);
                    initial_idx_=new_idx;
                    model_->fetch(new_idx);
                    found=true;
                    break;
                }
            }
            if(!found){
                initial_path_.clear();
            }
        }else{
            model_->fetch(initial_idx_);
        }
    }else{
        ui->tree->setCurrentIndex(proxy_->mapFromSource(initial_idx_));
        initial_idx_=QModelIndex();
        disconnect(model_,&QSsh::SftpFileSystemModel::dirListed,this,&RemoteFileDialog::modelReady_);
    }
}

void RemoteFileDialog::initSftpFileSystemModel_()
{
    if(model_){
        model_->shutDown();
        model_->deleteLater();
    }
    model_=new QSsh::SftpFileSystemModel(this);
    proxy_->setSourceModel(model_);
    connect(model_,&QSsh::SftpFileSystemModel::connectionError,this,&RemoteFileDialog::onConnectionError);
    connect(model_,&QSsh::SftpFileSystemModel::connectionEstablished,this,&RemoteFileDialog::onConnectionEstablished);
    connect(model_,&QSsh::SftpFileSystemModel::dirListed,this,&RemoteFileDialog::modelReady_);
    ui->tree->setModel(proxy_);
    proxy_->sort(1);
}

void RemoteFileDialog::tryNextAuth_()
{
    //connect
    ui->message->setText("Connecting ...");
    remote_path_.setHost(ui->host->text());
    remote_path_.setUserName(ui->user->text());
    remote_path_.setPort(ui->port->text().toInt());
    SshAuthenticationStore store;
    if(store.contains(remote_path_.toConnectionParameters()) && !stored_connection_){
        default_keys_.clear();
        model_->setSshConnection(store.retrieve(remote_path_.toConnectionParameters()));
        stored_connection_=true;
    }else{
        stored_connection_=false;
        if(!default_keys_.empty()){
            QString next_key=default_keys_.back();
            default_keys_.pop_back();
            remote_path_.setAuthType(QSsh::SshConnectionParameters::AuthenticationByKey);
            remote_path_.setKey(next_key);
            model_->setSshConnection(remote_path_.toConnectionParameters());
        }else{
            SshAuthenticationDialog::auth_type auth=SshAuthenticationDialog::getSshAuthentication(QString("Authentication for: %1").arg(remote_path_.toString(QUrl::RemovePassword)));
            if(auth.second!=""){
                remote_path_.setAuthType(auth.first);
                if(auth.first==QSsh::SshConnectionParameters::AuthenticationByPassword){
                    remote_path_.setPassword(auth.second);
                }else{
                    remote_path_.setKey(auth.second);
                }
                model_->setSshConnection(remote_path_.toConnectionParameters());
            }else{
                disconnect_();
                ui->message->setText("Disconnected");
            }
        }
    }
}

void RemoteFileDialog::disconnect_()
{
    initSftpFileSystemModel_();
    ui->message->setText("Disconnected");
    ui->connect->setText("Connect");
    ui->connect->setChecked(false);
}
