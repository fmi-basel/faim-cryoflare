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
#ifndef EPUDATASOURCE_H
#define EPUDATASOURCE_H

#include <QScopedPointer>
#include <QDir>
#include "datasourcebase.h"

//fw decl
class FileSystemWatcher;

class EPUDataSource : public DataSourceBase
{
    Q_OBJECT
public:
    EPUDataSource(MetaDataStore *store);
    virtual ~EPUDataSource();
public slots:
    virtual void start();
    virtual void stop();
    virtual void setProjectDir(const QString& epu_project_dir);
    virtual void setMovieDir(const QString& movie_dir);
protected slots:
    virtual void onDirChanged(const QString & path);
protected:
    void parseGridSquareXMLs_(const QDir &dir);
    void parseGridSquareDMs_(const QDir &dir);
    void parseTargetLocations_(const QDir &dir);
    void parseMicrographs_(const QDir &dir);
    void addSubdirectories_(const QDir& directory, const QStringList& subdirs);
    QScopedPointer<FileSystemWatcher> watcher_;
    QString epu_project_dir_;
    QString movie_dir_;
    QStringList xml_files_;
    QHash<QString,Data> grid_square_data_;
    QHash<QString,Data> foil_hole_data_;

};

#endif // EPUDATASOURCE_H
