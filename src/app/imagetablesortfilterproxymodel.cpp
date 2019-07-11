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

#include <QColor>
#include <QBrush>
#include "imagetablesortfilterproxymodel.h"

ImageTableSortFilterProxyModel::ImageTableSortFilterProxyModel(QObject *parent):
    QSortFilterProxyModel(parent)
{

}

QVariant ImageTableSortFilterProxyModel::data(const QModelIndex &index, int role) const
{
    if(role==Qt::BackgroundRole && index.row()%2==0){
        return QBrush(QSortFilterProxyModel::data(index,role).value<QBrush>().color().darker(110));
    }else{
        return QSortFilterProxyModel::data(index,role);
    }
}
