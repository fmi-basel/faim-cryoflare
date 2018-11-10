#ifndef MEATDATASTORE_H
#define MEATDATASTORE_H

#include <QObject>
#include <QVector>
#include <QScopedPointer>
#include "dataptr.h"

//fw decl
class DataSourceBase;

class MetaDataStore : public QObject
{
    Q_OBJECT
public:
    explicit MetaDataStore(QObject *parent = nullptr);
    ~MetaDataStore();
    void setDataSource(DataSourceBase *source);

signals:
    void newImage(const DataPtr & ptr);
protected slots:
    void addImage(const DataPtr & ptr);
protected:
    QScopedPointer<DataSourceBase> data_source_;
    QVector<DataPtr> data_;
};

#endif // MEATDATASTORE_H
