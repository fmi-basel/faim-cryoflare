#include <QColor>
#include <QBrush>
#include "imagetablesortfilterproxymodel.h"

ImageTableSortFilterProxyModel::ImageTableSortFilterProxyModel(QObject *parent):
    QSortFilterProxyModel(parent)
{

}

QVariant ImageTableSortFilterProxyModel::data(const QModelIndex &index, int role) const
{
    if(role==Qt::BackgroundRole && index.row()%2==0){
        return QBrush(QSortFilterProxyModel::data(index,role).value<QBrush>().color().darker(110));
    }else{
        return QSortFilterProxyModel::data(index,role);
    }
}
