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
#ifndef REMOTEPATHEDIT_H
#define REMOTEPATHEDIT_H

#include "pathedit.h"
#include <QUrl>

class RemotePathEdit : public PathEdit
{
    Q_OBJECT
public:
    RemotePathEdit(QWidget *parent = 0);
    QUrl remotePath() const;
    void setRemotePath(const QUrl &path);

signals:
    void pathChanged(QUrl);
public slots:
    void onRemoteBrowse();

private:
    QPushButton *remote_browse_;
    QUrl remote_path_;
private slots:
    void updateUrl(const QString& text);
};

#endif // REMOTEPATHEDIT_H
