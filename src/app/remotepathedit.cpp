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
    connect(path_widget_,&QLineEdit::textEdited,this,&RemotePathEdit::updateUrl);
}

QUrl RemotePathEdit::remotePath() const
{
    return remote_path_;
}

void RemotePathEdit::setRemotePath(const QUrl &path)
{
    remote_path_=path;
    path_widget_->setText(path.toString(QUrl::RemovePassword|QUrl::PreferLocalFile|QUrl::NormalizePathSegments));
}

void RemotePathEdit::onRemoteBrowse()
{
    // todo implement path_types for remote locations ( probably in SFTPFilesystemModel)
    switch(path_type_){
    case ExistingDirectory:
        break;
    case OpenFileName:
        break;
    case SaveFileName:
        break;
    }

    QUrl sanitized_path=remote_path_;
    sanitized_path.setScheme("sftp");
    if(sanitized_path.port()==-1){
        sanitized_path.setPort(22);
    }
    QUrl new_path=RemoteFileDialog::getRemotePath(sanitized_path);
    if(new_path.isValid()){
        setRemotePath(new_path);
    }

}

void RemotePathEdit::updateUrl(const QString &text)
{
    QUrl new_url=QUrl::fromUserInput(text);
    if(new_url.isValid()){
        remote_path_=new_url;
    }
}
