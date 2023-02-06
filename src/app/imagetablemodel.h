//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2017-2019 by the CryoFLARE Authors
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

//fw decl
class MetaDataStore;
class TaskConfiguration;

class ImageTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Role {
        SortRole=Qt::UserRole,
        SummaryRole,
        VisibilityRole
    };
    ImageTableModel(MetaDataStore* store,TaskConfiguration* task_config, QObject * parent = nullptr);
    virtual int rowCount(const QModelIndex &parent=QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    Data image(int row);
    QString id(int row);
public slots:
    void onMicrographUpdated(const QString &id, const QStringList &keys);
    void onMicrographAdded(const QString &id);
    void onTasksChanged();

private:
    MetaDataStore* meta_data_store_;
    QVector<QString> micrograph_id_;
    QList<InputOutputVariable>  columns_;
    QList<QColor> colors_;
    TaskConfiguration* task_configuration_;

};

#endif // IMAGETABLEMODEL_H
