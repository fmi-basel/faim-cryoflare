#ifndef IMAGETABLEMODEL_H
#define IMAGETABLEMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <QMap>
#include <QStringList>
#include <QPair>
#include <dataptr.h>

class ImageTableModel : public QAbstractTableModel
{
public:
    ImageTableModel(QObject * parent = 0);
    virtual int rowCount(const QModelIndex &parent=QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    DataPtr image(int row);
    void addImage(const DataPtr & data);
    void addColumn(const QPair<QString,QString>  &column);
    void clearColumns();
    void onDataChanged(const DataPtr &data);
private:
    QList<DataPtr> data_;
    QPair<QStringList,QStringList>  columns_;

};

#endif // IMAGETABLEMODEL_H
