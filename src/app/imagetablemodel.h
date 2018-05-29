//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFlare
//
// Copyright (C) 2017-2018 by the CryoFlare Authors
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
// along with CryoFlare.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------------

#ifndef IMAGETABLEMODEL_H
#define IMAGETABLEMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <QColor>
#include <QMap>
#include <QStringList>
#include <QPair>
#include <dataptr.h>
#include <inputoutputvariable.h>

class ImageTableModel : public QAbstractTableModel
{
public:
    enum Role {
        SortRole=Qt::UserRole,
        SummaryRole
    };
    ImageTableModel(QObject * parent = 0);
    virtual int rowCount(const QModelIndex &parent=QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    DataPtr image(int row);
    void addImage(const DataPtr & data);
    void addColumn(const InputOutputVariable  &column, const QColor& color=QColor(255,255,255));
    void clearColumns();
    void onDataChanged(const DataPtr &data);
    void clearData();
private:
    QList<DataPtr> data_;
    QList<InputOutputVariable>  columns_;
    QList<QColor> colors_;

};

#endif // IMAGETABLEMODEL_H
