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
#include "sftpfilesystemmodel.h"
#include "ui_remotefiledialog.h"

RemoteFileDialog::RemoteFileDialog(const QUrl &remote_path, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RemoteFileDialog),
    model_(new SFTPFileSystemModel(this)),
    proxy_(new QSortFilterProxyModel(this)),
    remote_path_(remote_path)
{
    ui->setupUi(this);
    ui->tree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    if(remote_path.isValid()){
        ui->host->setText(remote_path.host());
        ui->user->setText(remote_path.userName());
        ui->port->setText(QString("%1").arg(remote_path.port()));
    }else{
        ui->port->setText(QString("%1").arg(22));
    }
    remote_path_.setScheme("sftp");
}

QUrl RemoteFileDialog::remotePath() const
{
    QModelIndex idx=ui->tree->currentIndex();
    QString path=proxy_->data(proxy_->index( idx.row(),1,idx.parent()),SFTPFileSystemModel::PathRole).toString();
    if(path.isEmpty()){
        return QUrl();
    }
    QUrl remote_path=remote_path_;
    remote_path.setPath(path);
    return remote_path;
}


QUrl RemoteFileDialog::getRemotePath(const QUrl &path )
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

void RemoteFileDialog::connect(bool con)
{
    if(con){
        if(model_->connect(remote_path_)){
            ui->message->setText("Connected");
            ui->connect->setText("Disconnect");
            remote_path_=model_->getUrl();
            proxy_->setSourceModel(model_);
            proxy_->setFilterRegExp("^[^\\.].*"); // filter out hidden files
            proxy_->setFilterKeyColumn(1);
            proxy_->setSortRole(SFTPFileSystemModel::SortRole);
            ui->tree->setSortingEnabled(true);
            ui->tree->setModel(proxy_);
            proxy_->sort(1);
            QModelIndex idx=model_->pathToIndex(remote_path_.path());
            if(idx.isValid()){
                ui->tree->setCurrentIndex(proxy_->mapFromSource(idx));
            }

        }else{
            model_->disconnect();
            ui->message->setText("Connection error");
            ui->connect->setText("Connect");
            ui->connect->setChecked(false);
         }
    }else{
        model_->disconnect();
        ui->message->setText("Disconnected");
        ui->connect->setText("Connect");
        ui->connect->setChecked(false);
    }
}

