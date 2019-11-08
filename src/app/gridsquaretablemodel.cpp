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
    return gridsquare_id_.size();
}

int GridsquareTableModel::columnCount(const QModelIndex &parent) const
{
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
