#ifndef TABLESUMMARYMODEL_H
#define TABLESUMMARYMODEL_H

#include <QTimer>
#include "imagetablemodel.h"

class TableSummaryModel : public QAbstractTableModel
{
public:
    TableSummaryModel(ImageTableModel * model,QObject * parent = nullptr);
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual int rowCount(const QModelIndex &parent=QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent=QModelIndex()) const;
public slots:
    void sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void update();
protected:
    ImageTableModel* model_;
    QTimer update_timer_;
};

#endif // TABLESUMMARYMODEL_H
