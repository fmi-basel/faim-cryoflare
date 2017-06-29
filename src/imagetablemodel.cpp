#include <QtDebug>
#include "imagetablemodel.h"

ImageTableModel::ImageTableModel(QObject *parent):
    QAbstractTableModel(parent)
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
    // handle export check box
    if(0==index.column()){
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
    // handle other columns
    if(role==SortRole){
        switch(columns_[index.column()-1].type){
        case String:
            return data_.at(index.row())->value(columns_[index.column()-1].label);
            break;
        case Float:
            return data_.at(index.row())->value(columns_[index.column()-1].label).toDouble();
            break;
        case Int:
            return data_.at(index.row())->value(columns_[index.column()-1].label).toInt();
            break;
        case Image:
        default:
            return "-";
            break;
        }
    }else if(role==Qt::DisplayRole){
        return data_.at(index.row())->value(columns_[index.column()-1].label);
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

void ImageTableModel::addColumn(const InputOutputVariable &column)
{
    beginResetModel();
    columns_.append(column);
    endResetModel();
}

void ImageTableModel::clearColumns()
{
    if(! columns_.empty()){
        beginResetModel();
        columns_.clear();
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
