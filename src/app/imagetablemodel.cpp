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

#include <QBrush>
#include <QtDebug>
#include "metadatastore.h"
#include "imagetablemodel.h"

ImageTableModel::ImageTableModel(MetaDataStore* store, TaskConfiguration *task_config, QObject *parent):
    QAbstractTableModel(parent),
    meta_data_store_(store),
    micrograph_id_(),
    columns_(),
    colors_(),
    task_configuration_(task_config)
{
    connect(store,&MetaDataStore::newMicrograph,this , &ImageTableModel::onMicrographAdded);
    connect(store,&MetaDataStore::micrographUpdated,this , &ImageTableModel::onMicrographUpdated);
    connect(task_config,&TaskConfiguration::configurationChanged,this , &ImageTableModel::onTasksChanged);
    foreach(QString id, store->micrographIDs()){
        beginInsertRows(QModelIndex(),rowCount(),rowCount());
        micrograph_id_.append(id);
        endInsertRows();
    }
    onTasksChanged();

}

int ImageTableModel::rowCount(const QModelIndex &/*parent*/) const
{
    return micrograph_id_.size();
}

int ImageTableModel::columnCount(const QModelIndex &/*parent*/) const
{
    return columns_.size()+1;
}

QVariant ImageTableModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()){
        return QVariant();
    }
    // handle export check box
    if(0==index.column()){
        if(role==Qt::BackgroundRole){
            return QBrush(QColor(136, 138, 133));
        }
        QString value=meta_data_store_->micrograph(micrograph_id_.value(index.row())).value("export").toString("true");
        bool state=value.compare("true", Qt::CaseInsensitive) == 0 || value==QString("1");
        if(role==Qt::CheckStateRole){
            if (state)
            {
                return Qt::Checked;
            }else{
                return Qt::Unchecked;
            }
        }
        if(role==SortRole){
            if(state){
                return 1;
            }else{
                return 0;
            }
        }
        return QVariant();
    }
    if(role==Qt::BackgroundRole){
        int col=index.column();
        if(col>colors_.size()){
            return QBrush(QColor(136, 138, 133));
        }else{
            return QBrush(colors_[col-1]);
        }

    }
    // handle other columns
    if(role==SortRole){
        QVariant v=QVariant(meta_data_store_->micrograph(micrograph_id_.value(index.row())).value(columns_[index.column()-1].label).toString());
        switch(columns_[index.column()-1].type){
        case String:
            return v;
        case Float:
            if(v.toString()==QString("")){
                return QVariant();
            }
            return v.toDouble();
        case Int:
            if( v.toString()==QString("")){
                return QVariant();
            }
            return v.toInt();
        case Image:
        default:
            return "-";
        }
    }else if(role==Qt::DisplayRole){
        return meta_data_store_->micrograph(micrograph_id_.value(index.row())).value(columns_[index.column()-1].label).toString();
    }else if (role==SummaryRole){
        return columns_[index.column()-1].summary_type;
    }
    return QVariant();
}

bool ImageTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(role==Qt::CheckStateRole && index.column()==0){
        meta_data_store_->setMicrographExport(micrograph_id_.value(index.row()),value == Qt::Checked);
        return true;
    }else{
        return false;
    }
}

QVariant ImageTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role==VisibilityRole){
        if(orientation==Qt::Vertical){
            return true;
        }else{
            if(section==0){
                return true;
            }
            return columns_.value(section-1).in_column;
        }
    }
    if(role==Qt::DisplayRole){
        if(orientation==Qt::Horizontal){
            if(section==0){
                return "Export";
            }else{
                if(columns_.size()>section-1 && section>0){
                    return QString("%1").arg(columns_[section-1].key);
                }else{
                   return "invalid";
                }
            }
        }else{
            return section;
        }
    } else if(role==Qt::BackgroundRole){
        if(section>colors_.size() || section<1 || orientation==Qt::Vertical){
            return QBrush(QColor(136, 138, 133));
        }else{
            return QBrush(colors_[section-1]);
        }

    }else if (role==SummaryRole){
        if(section>0 && section <=columns_.size()){
            return columns_[section-1].summary_type;
        }else{
            return QVariant(0);
        }
    }else{
        return QAbstractTableModel::headerData(section,orientation,role);
    }
}

Qt::ItemFlags ImageTableModel::flags(const QModelIndex &index) const
{
    if(0==index.column()){
        return QAbstractTableModel::flags(index) | Qt::ItemIsUserCheckable;
    }else{
        return QAbstractTableModel::flags(index);
    }
}


Data ImageTableModel::image(int row)
{
    return meta_data_store_->at(row);
}

QString ImageTableModel::id(int row)
{
    return micrograph_id_[row];
}


void ImageTableModel::onMicrographAdded(const QString &id)
{
   beginInsertRows(QModelIndex(),rowCount(),rowCount());
   micrograph_id_.append(id);
   endInsertRows();
}

void ImageTableModel::onTasksChanged()
{
    beginResetModel();
    columns_.clear();
    colors_.clear();
    QList<TaskDefinitionPtr> def_list;
    def_list.append(task_configuration_->rootDefinition());
    //Warning: ordering here has to match code in TaskConfiguration
    //Todo: merge code handling IO variables and color into TaskConfiguration
    while(!def_list.empty()){
        TaskDefinitionPtr ptr=def_list.takeFirst();
        columns_.append(ptr->result_variables_);
        for(int i=0;i<ptr->result_variables_.size();++i){
            colors_.append(ptr->color);
        }
        QList<TaskDefinitionPtr> children=ptr->children;
        while (!children.empty()) {
            def_list.push_front(children.takeLast());
        }
    }
    endResetModel();
}


void ImageTableModel::onMicrographUpdated(const QString &id)
{
    int idx=micrograph_id_.indexOf(id);
    if(idx>=0){
        dataChanged(index(idx,0),index(idx,columns_.size()));
    }
}

