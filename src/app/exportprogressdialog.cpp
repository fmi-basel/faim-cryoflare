//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2019 by the CryoFLARE Authors
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
#include "exportprogressdialog.h"
#include "ui_exportprogressdialog.h"

ExportProgressDialog::ExportProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportProgressDialog)
{
    ui->setupUi(this);
    setModal(true);
    hide();
}

ExportProgressDialog::~ExportProgressDialog()
{
    delete ui;
}

void ExportProgressDialog::start(const QString &title, int num)
{
    setWindowTitle(title);
    ui->progress->reset();
    ui->progress->setMaximum(num);
    ui->finish->hide();
    ui->cancel->show();
    ui->details->clear();
    open();
}

void ExportProgressDialog::update(const QList<ExportMessage> &messages, int num_left)
{
    ui->progress->setValue(ui->progress->maximum()-num_left);
    update(messages);
}

void ExportProgressDialog::update(const QList<ExportMessage> &messages)
{
    QString new_messages;
    foreach( ExportMessage m,messages){
        if(m.type==ExportMessage::ERROR){
            new_messages+=QString("<font color=red><b>%1: %2</b></font><br>").arg(m.id).arg(m.text);
        }else{
            new_messages+=QString("%1: %2<br>").arg(m.id).arg(m.text);
        }
    }
    ui->details->setText(ui->details->text()+new_messages);
}

void ExportProgressDialog::finish()
{
    ui->finish->show();
    ui->cancel->hide();
}
