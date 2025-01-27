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
#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include <QProgressDialog>
#include <libssh/libssh.h>

namespace Ui {
class ExportDialog;
}

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(const QStringList& raw_keys, const QStringList& output_keys,const QStringList& shared_raw_keys,const QStringList& shared_keys, QWidget *parent = nullptr);
    ~ExportDialog();
    QUrl destinationPath() const;
    QUrl rawDestinationPath() const;
    void setDestinationPath(const QUrl& url);
    void setRawDestinationPath(const QUrl& url);
    bool separateRawPath() const;
    void setSeparateRawPath( bool f);
    bool duplicateRaw() const;
    void setDuplicateRaw( bool f);
    bool exportReportMetadata() const;
    void setExportReportMetadata( bool f);
    QStringList selectedOutputKeys() const;
    QStringList selectedRawKeys() const;
    QStringList selectedSharedKeys() const;
    QStringList selectedSharedRawKeys() const;

public slots:
    void verifyDestinations();
    //void destinationVerified();
    //void destinationVerificationError(QSsh::SshError e);
    //void rawDestinationVerificationError(QSsh::SshError e);


private:
    Ui::ExportDialog *ui;
};

#endif // EXPORTDIALOG_H
