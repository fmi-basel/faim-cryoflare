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
            if(columns_.second.size()>section){
                return QString("%1").arg(columns_.first[section]);
            }else{
               return "invalid";
            }

        }else{
            return section;
        }
    }else{
        return QAbstractTableModel::headerData(section,orientation,role);
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

void ImageTableModel::addColumn(const QPair<QString, QString> &column)
{
    beginResetModel();
    columns_.first.append(column.first);
    columns_.second.append(column.second);
    endResetModel();
}

void ImageTableModel::clearColumns()
{
    if(! columns_.first.empty()){
        beginResetModel();
        columns_.first.clear();
        columns_.second.clear();
        endResetModel();
    }
}

void ImageTableModel::onDataChanged(const DataPtr &data)
{
    int idx=data_.indexOf(data);
    if(idx>=0){
        dataChanged(index(idx,0),index(idx,columns_.first.size()-1));
    }
}
