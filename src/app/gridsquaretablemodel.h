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
#ifndef GRIDSQUARETABLEMODEL_H
#define GRIDSQUARETABLEMODEL_H

#include "metadatastore.h"
#include <QAbstractTableModel>

class GridsquareTableModel : public QAbstractTableModel
{
public:
    GridsquareTableModel(MetaDataStore* store, QObject *parent = nullptr);
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
public slots:
    void gridsquareAdded(const QString &id);
protected:
    MetaDataStore* metadata_store_;
    QVector<QString> gridsquare_id_;
};

#endif // GRIDSQUARETABLEMODEL_H
