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

#include <QBrush>
#include <QtDebug>
#include "imagetablemodel.h"

ImageTableModel::ImageTableModel(QObject *parent):
    QAbstractTableModel(parent),
    data_(),
    columns_(),
    colors_()
{

}

int ImageTableModel::rowCount(const QModelIndex &parent) const
{
    return data_.size();
}

int ImageTableModel::columnCount(const QModelIndex &parent) const
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
            return QBrush(QColor(255,255,255));
        }
        if(role!=Qt::CheckStateRole){
            return QVariant();
        }
        QString value=data_.at(index.row())->value("export","true");
        if (value.compare("true", Qt::CaseInsensitive) == 0 || value==QString("1"))
        {
            return Qt::Checked;
        }else{
            return Qt::Unchecked;
        }
    }
    if(role==Qt::BackgroundRole){
        int col=index.column();
        if(col>colors_.size()){
            return QBrush(QColor(255,255,255));
        }else{
            return QBrush(colors_[col-1]);
        }

    }
    // handle other columns
    if(role==SortRole){
        QVariant v=data_.at(index.row())->value(columns_[index.column()-1].label);
        switch(columns_[index.column()-1].type){
        case String:
            return v;
            break;
        case Float:
            if(v.toString()==QString("")){
                return QVariant();
            }
            return v.toDouble();
            break;
        case Int:
            if( v.toString()==QString("")){
                return QVariant();
            }
            return v.toInt();
            break;
        case Image:
        default:
            return "-";
            break;
        }
    }else if(role==Qt::DisplayRole){
        return data_.at(index.row())->value(columns_[index.column()-1].label);
    }else if (role==SummaryRole){
        return columns_[index.column()-1].summary_type;
    }
    return QVariant();
}

bool ImageTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(role==Qt::CheckStateRole && index.column()==0){
        if(value == Qt::Checked){
            data_.at(index.row())->insert("export","true");
        }else{
            data_.at(index.row())->insert("export","false");
        }
        emit dataChanged(index,index);
        return true;
    }else{
        return false;
    }
}

QVariant ImageTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role==Qt::DisplayRole){
        if(orientation==Qt::Horizontal){
            if(section==0){
                return "Export";
            }else{
                if(columns_.size()>section-1){
                    return QString("%1").arg(columns_[section-1].key);
                }else{
                   return "invalid";
                }
            }
        }else{
            return section;
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

DataPtr ImageTableModel::image(int row)
{
    return data_[row];
}

void ImageTableModel::addImage(const DataPtr &data)
{
    beginInsertRows(QModelIndex(),rowCount(QModelIndex()),rowCount(QModelIndex()));
    data_.append(data);
    endInsertRows();
}

void ImageTableModel::addColumn(const InputOutputVariable &column, const QColor &color)
{
    beginResetModel();
    columns_.append(column);
    colors_.append(color);
    endResetModel();
}

void ImageTableModel::clearColumns()
{
    if(! columns_.empty()){
        beginResetModel();
        columns_.clear();
        colors_.clear();
        endResetModel();
    }
}

void ImageTableModel::onDataChanged(const DataPtr &data)
{
    int idx=data_.indexOf(data);
    if(idx>=0){
        dataChanged(index(idx,0),index(idx,columns_.size()));
    }
}

void ImageTableModel::clearData()
{
    if(! data_.empty()){
        beginResetModel();
        data_.clear();
        endResetModel();
    }
}
