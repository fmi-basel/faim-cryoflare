#ifndef GRIDSQUARETABLEMODEL_H
#define GRIDSQUARETABLEMODEL_H

#include "metadatastore.h"
#include <QAbstractTableModel>

class GridsquareTableModel : public QAbstractTableModel
{
public:
    GridsquareTableModel(MetaDataStore* store, QObject *parent = nullptr);
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
public slots:
    void gridsquareAdded(const QString &id);
protected:
    MetaDataStore* metadata_store_;
    QVector<QString> gridsquare_id_;
};

#endif // GRIDSQUARETABLEMODEL_H
