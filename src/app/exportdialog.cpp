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
#include <QListWidget>
#include <QMessageBox>
#include "exportdialog.h"
#include <QInputDialog>
#include "sshsession.h"
#include "ui_exportdialog.h"

ExportDialog::ExportDialog(const QStringList &raw_keys, const QStringList &output_keys,const QStringList& shared_raw_keys, const QStringList &shared_keys, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportDialog)
{
    ui->setupUi(this);
    ui->data_path->setPathType(PathEdit::ExistingDirectory);
    ui->raw_data_path->setPathType(PathEdit::ExistingDirectory);
    ui->data_list->addItems(output_keys);
    ui->data_list->selectAll();
    ui->raw_data_list->addItems(raw_keys);
    ui->raw_data_list->selectAll();
    ui->shared_raw_data_list->addItems(shared_raw_keys);
    ui->shared_raw_data_list->selectAll();
    ui->shared_data_list->addItems(shared_keys);
    ui->shared_data_list->selectAll();
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

QUrl ExportDialog::destinationPath() const
{
    return ui->data_path->remotePath();
}

QUrl ExportDialog::rawDestinationPath() const
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

bool ExportDialog::exportReportMetadata() const
{
    return ui->export_report_metadata->isChecked();
}

void ExportDialog::setExportReportMetadata(bool f)
{
    ui->export_report_metadata->setChecked(f);
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

QStringList ExportDialog::selectedSharedRawKeys() const
{
    QStringList result;
    foreach(QListWidgetItem* i, ui->shared_raw_data_list->selectedItems()){
        result << i->text();
    }
    return result;
}

void ExportDialog::verifyDestinations()
{
    if(!destinationPath().isLocalFile()){
        SSHSession session=SSHSession::createAuthenticatedSession(destinationPath());
        if(session.isConnected()){
            setDestinationPath(session.getUrl());
        }else{
            return;
        }
    }
    if(separateRawPath() && !rawDestinationPath().isLocalFile()){
        SSHSession session=SSHSession::createAuthenticatedSession(rawDestinationPath());
        if(session.isConnected()){
            setRawDestinationPath(session.getUrl());
        }else{
            return;
        }
    }
    accept();
}

