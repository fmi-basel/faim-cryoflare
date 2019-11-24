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
#ifndef EXPORTPROGRESSDIALOG_H
#define EXPORTPROGRESSDIALOG_H

#include <QDialog>
#include "parallelexporter.h"
namespace Ui {
class ExportProgressDialog;
}

class ExportProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportProgressDialog(QWidget *parent = nullptr);
    ~ExportProgressDialog();
public slots:
    void start(const QString& title,int num);
    void update(const QList<ExportMessage>& messages,int num_left);
    void update(const QList<ExportMessage>& messages);
    void finish();

private:
    Ui::ExportProgressDialog *ui;
};

#endif // EXPORTPROGRESSDIALOG_H
