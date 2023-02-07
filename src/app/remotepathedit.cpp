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
#include <QPushButton>
#include <QIcon>
#include <QLayout>
#include <QLineEdit>
#include "remotepathedit.h"
#include "remotefiledialog.h"

RemotePathEdit::RemotePathEdit(QWidget *parent):
    PathEdit(parent),
    remote_browse_(new QPushButton(QIcon(":/icons/document-open-remote.png"),"",this)),
    remote_path_()
{
    layout()->addWidget(remote_browse_);
    remote_browse_->setToolTip("Browse remote folder");
    connect(remote_browse_, &QPushButton::clicked,this,&RemotePathEdit::onRemoteBrowse);
    connect(path_widget_,&QLineEdit::textChanged,this,&RemotePathEdit::updateUrl);
}

SftpUrl RemotePathEdit::remotePath() const
{
    return remote_path_;
}

void RemotePathEdit::setRemotePath(const SftpUrl &path)
{
    remote_path_=path;
    path_widget_->setText(path.toString(QUrl::RemovePassword));
}

void RemotePathEdit::onRemoteBrowse()
{
    // todo implement path_types for remote locations ( probably in SftpFilesystemModel)
    switch(path_type_){
    case ExistingDirectory:
        break;
    case OpenFileName:
        break;
    case SaveFileName:
        break;
    }


    SftpUrl new_path=RemoteFileDialog::getRemotePath(remote_path_);
    if(new_path.isValid()){
        path_widget_->setText(new_path.toString(QUrl::RemovePassword));
        remote_path_=new_path;
    }

}

void RemotePathEdit::updateUrl(const QString &text)
{
    SftpUrl new_url=QUrl::fromUserInput(text);
    if(new_url.isValid()){
        if( !new_url.isLocalFile()){
            if(remote_path_.authType()==QSsh::SshConnectionParameters::AuthenticationByPassword){
                new_url.setPassword(remote_path_.password());
            }else{
                new_url.setKey(remote_path_.key());
            }
        }
        remote_path_=new_url;
    }
}
