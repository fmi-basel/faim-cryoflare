#ifndef IMAGETABLEMODEL_H
#define IMAGETABLEMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <QMap>
#include <QStringList>
#include <QPair>
#include <dataptr.h>
#include <inputoutputvariable.h>

class ImageTableModel : public QAbstractTableModel
{
public:
    enum Role {
        SortRole=Qt::UserRole
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
    void addColumn(const InputOutputVariable  &column);
    void clearColumns();
    void onDataChanged(const DataPtr &data);
    void clearData();
private:
    QList<DataPtr> data_;
    QList<InputOutputVariable>  columns_;

};

#endif // IMAGETABLEMODEL_H
