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
#include <QUrl>

//fw decl
class SFTPFileSystemModel;
namespace Ui {
class RemoteFileDialog;
}

class RemoteFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RemoteFileDialog(const QUrl& remote_path=QUrl(),QWidget *parent = nullptr);
    QUrl remotePath() const;
    static QUrl getRemotePath(const QUrl& path );
    ~RemoteFileDialog();
public slots:
    void connect(bool con);
private:
    Ui::RemoteFileDialog *ui;
    SFTPFileSystemModel* model_;
    QSortFilterProxyModel* proxy_;
    QUrl remote_path_;
};

#endif // REMOTEFILEDIALOG_H
