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
#ifndef REMOTEFILEDIALOG_H
#define REMOTEFILEDIALOG_H

#include <QDialog>
#include <QSortFilterProxyModel>
#include <sftpurl.h>

//fw decl
namespace QSsh {
class SftpFileSystemModel;
} //ns Qssh

namespace Ui {
class RemoteFileDialog;
}

class RemoteFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RemoteFileDialog(const SftpUrl& remote_path=SftpUrl(),QWidget *parent = nullptr);
    SftpUrl remotePath() const;
    static SftpUrl getRemotePath(const SftpUrl& path );
    ~RemoteFileDialog();
public slots:
    void connectToHost(bool con);
    void onConnectionEstablished();
    void onConnectionError(const QString &error);
protected slots:
    void modelReady_();
private:
    void initSftpFileSystemModel_();
    void tryNextAuth_();
    void disconnect_();
    Ui::RemoteFileDialog *ui;
    QSsh::SftpFileSystemModel* model_;
    QSortFilterProxyModel* proxy_;
    SftpUrl remote_path_;
    QStringList initial_path_;
    QModelIndex initial_idx_;
    QStringList default_keys_;
    bool stored_connection_;
};

#endif // REMOTEFILEDIALOG_H
