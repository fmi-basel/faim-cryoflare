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

#ifndef PATHEDIT_H
#define PATHEDIT_H

#include <QWidget>

//fw decl
class QLineEdit;
class QPushButton;

class PathEdit : public QWidget
{
    Q_OBJECT
public:
    enum PathType{
        ExistingDirectory,
        OpenFileName,
        SaveFileName
    };
    PathEdit( QWidget *parent = nullptr);
    explicit PathEdit(PathType t=OpenFileName, QString caption=QString(), QString path=QString(), QString filter=QString(), QWidget *parent = nullptr);
    QString path() const;
    void setPath(const QString &path);

    PathType pathType() const;
    void setPathType(const PathType &pathType);

signals:
    void pathChanged(QString);
public slots:
    void onBrowse();

protected:
    PathType path_type_;
    QString caption_;
    QString filter_;
    QLineEdit *path_widget_;
    QPushButton *browse_;

};

#endif // PATHEDIT_H
