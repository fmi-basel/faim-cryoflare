#include "metadatastore.h"
#include "datasourcebase.h"
#include "epudatasource.h"

MetaDataStore::MetaDataStore(QObject *parent):
    QObject(parent),
    data_source_(nullptr),
    data_()
{
}

MetaDataStore::~MetaDataStore()
{

}

void MetaDataStore::setDataSource(DataSourceBase *source)
{
    data_source_.reset(source);
    connect(data_source_.data(),&DataSourceBase::newImage,this,&MetaDataStore::addImage);
}

void MetaDataStore::addImage(const DataPtr &ptr)
{
    data_.append(ptr);
    emit newImage(ptr);
}
