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
#ifndef FLATFOLDERDATASOURCE_H
#define FLATFOLDERDATASOURCE_H

#include <QFileInfo>
#include <QScopedPointer>
#include "datasourcebase.h"

//fw decl
class FileSystemWatcher;
class QTimer;

class FlatFolderDataSource : public DataSourceBase
{
    Q_OBJECT
public:
    FlatFolderDataSource(MetaDataStore *store,const QString& pattern, bool xml=true);
    virtual ~FlatFolderDataSource();
public slots:
    virtual void start();
    virtual void stop();
    virtual void setProjectDir(const QString& project_dir);
    virtual void setMovieDir(const QString& movie_dir);
protected slots:
    virtual void onDirChanged(const QString & path);
    virtual void checkForFileChanges();
protected:
    Data readJson_(const QString & path);
    QScopedPointer<FileSystemWatcher> watcher_;
    QString project_dir_;
    QString movie_dir_;
    QStringList image_files_;
    QHash<QString,Data> grid_square_data_;
    QString pattern_;
    bool xml_;
    QTimer* check_file_timer_;
    QHash<QString,QPair<QDateTime,qint64> > watched_files_;
};



#endif // FLATFOLDERDATASOURCE_H
