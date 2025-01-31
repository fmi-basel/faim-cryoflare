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
#include "gridsquaretablemodel.h"

GridsquareTableModel::GridsquareTableModel(MetaDataStore* store, QObject *parent):
    QAbstractTableModel (parent),
    metadata_store_(store),
    gridsquare_id_()
{
    connect(metadata_store_,&MetaDataStore::newGridsquare,this,&GridsquareTableModel::gridsquareAdded);
}

int GridsquareTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return gridsquare_id_.size();
}

int GridsquareTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant GridsquareTableModel::data(const QModelIndex &index, int role) const
{
    if(role!= Qt::DisplayRole){
        return QVariant();
    }else{
        return metadata_store_->gridsquareAt(index.row()).value("id").toString();
    }
}

void GridsquareTableModel::gridsquareAdded(const QString &id)
{
    beginInsertRows(QModelIndex(),gridsquare_id_.size(),gridsquare_id_.size());
    gridsquare_id_.append(id);
    endInsertRows();
}
