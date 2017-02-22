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
    return columns_.first.size();
}

QVariant ImageTableModel::data(const QModelIndex &index, int role) const
{
    if(role!=Qt::DisplayRole){
        return QVariant();
    }else{
        return data_.at(index.row())->value(columns_.first[index.column()]);
    }
}

QVariant ImageTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role==Qt::DisplayRole){
        if(orientation==Qt::Horizontal){
            return QString("%1").arg(columns_.second[section]);
        }else{
            return section;
        }
    }else{
        return QAbstractTableModel::headerData(section,orientation,role);
    }
}

void ImageTableModel::addImage(const DataPtr &data)
{
    beginInsertRows(QModelIndex(),rowCount(QModelIndex()),rowCount(QModelIndex()));
    data_.append(data);
    endInsertRows();
}

void ImageTableModel::setColumns(const QPair<QStringList, QStringList> &columns)
{
    beginRemoveColumns(QModelIndex(),0,columns_.first.size()-1);
    endRemoveColumns();
    beginInsertColumns(QModelIndex(),0,columns.first.size()-1);
    columns_=columns;
    endInsertColumns();
}

void ImageTableModel::onDataChanged(const DataPtr &data)
{
    int idx=data_.indexOf(data);
    if(idx>=0){
        dataChanged(index(idx,0),index(idx,columns_.first.size()-1));
    }
}
