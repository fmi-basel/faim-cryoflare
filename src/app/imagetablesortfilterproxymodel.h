#ifndef IMAGETABLESORTFILTERPROXYMODEL_H
#define IMAGETABLESORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class ImageTableSortFilterProxyModel : public QSortFilterProxyModel
{
public:
    ImageTableSortFilterProxyModel(QObject * parent = 0);
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
};

#endif // IMAGETABLESORTFILTERPROXYMODEL_H
