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
#ifndef DATASOURCEBASE_H
#define DATASOURCEBASE_H

#include <QObject>
#include "dataptr.h"

//fw decl
class MetaDataStore;
class DataSourceBase : public QObject
{
    Q_OBJECT
public:
    explicit DataSourceBase(MetaDataStore *parent);
    virtual ~DataSourceBase();
    MetaDataStore * store_;

signals:
    void newMicrograph(const Data& ptr);
    void newGridsquare(const Data& ptr);
    void newFoilhole(const Data& ptr);

public slots:
    virtual void start()=0;
    virtual void stop()=0;
    virtual void setProjectDir(const QString& epu_project_dir)=0;
    virtual void setMovieDir(const QString& movie_dir)=0;
};

#endif // DATASOURCEBASE_H
