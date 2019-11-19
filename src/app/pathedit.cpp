//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2017-2018 by the CryoFLARE Authors
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

#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include "pathedit.h"


PathEdit::PathEdit(QWidget *parent):
    QWidget(parent),
    path_type_(OpenFileName),
    caption_(),
    filter_(),
    path_widget_(new QLineEdit(this)),
    browse_(new QPushButton(QIcon(":/icons/document-open-folder.png"),"",this))
{
    QHBoxLayout *layout=new QHBoxLayout();
    layout->addWidget(path_widget_);
    layout->addWidget(browse_);
    setLayout(layout);
    connect(browse_,SIGNAL(clicked()),this,SLOT(onBrowse()));
    connect(path_widget_,SIGNAL(textChanged(QString)),this,SIGNAL(pathChanged(QString)));
    browse_->setToolTip("Browse folder");
}

PathEdit::PathEdit(PathEdit::PathType t, QString caption, QString path, QString filter, QWidget *parent):
    QWidget(parent),
    path_type_(t),
    caption_(caption),
    filter_(filter),
    path_widget_(new QLineEdit(path,this)),
    browse_(new QPushButton(QIcon(":/icons/document-open-folder.png"),"",this))
{
    QHBoxLayout *layout=new QHBoxLayout();
    layout->addWidget(path_widget_);
    layout->addWidget(browse_);
    this->setLayout(layout);
    connect(browse_,SIGNAL(clicked()),this,SLOT(onBrowse()));
    connect(path_widget_,SIGNAL(textChanged(QString)),this,SIGNAL(pathChanged(QString)));
}

QString PathEdit::path() const
{
    return path_widget_->text();
}

void PathEdit::setPath(const QString &path)
{
    path_widget_->setText(path);
}

void PathEdit::onBrowse()
{
    QString new_path;
    switch(path_type_){
    case ExistingDirectory:
        new_path = QFileDialog::getExistingDirectory(nullptr, caption_,path(),  QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);
        break;
    case OpenFileName:
        new_path = QFileDialog::getOpenFileName(nullptr, caption_,path(),filter_);
        break;
    case SaveFileName:
        new_path = QFileDialog::getSaveFileName(nullptr, caption_,path(),filter_);
        break;
    }
    if(! new_path.isEmpty()){
        path_widget_->setText(new_path);
    }
}

PathEdit::PathType PathEdit::pathType() const
{
    return path_type_;
}

void PathEdit::setPathType(const PathEdit::PathType &path_type)
{
    path_type_ = path_type;
}

void PathEdit::setIconSize(int size)
{
    browse_->setIconSize(QSize(size,size));
}
