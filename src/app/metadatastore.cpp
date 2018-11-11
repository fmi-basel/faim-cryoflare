#include <QDir>
#include <QDebug>
#include <QJsonDocument>
#include "settings.h"
#include "metadatastore.h"
#include "datasourcebase.h"
#include "epudatasource.h"

MetaDataStore::MetaDataStore(QObject *parent):
    QObject(parent),
    data_source_(nullptr),
    data_()
{
    QDir cryoflare_dir(CRYOFLARE_DIRECTORY);
    foreach(QString file_entry, cryoflare_dir.entryList(QStringList("*.dat"),QDir::Files,QDir::Time | QDir::Reversed)){
        QFile load_file(cryoflare_dir.filePath(file_entry));
        if(!load_file.open(QIODevice::ReadOnly)){
            qWarning() << "Couldn't open file: " << cryoflare_dir.filePath(file_entry);
            continue;
        }
        QJsonDocument load_doc=QJsonDocument::fromBinaryData(load_file.readAll());
        addImage(DataPtr(new Data(load_doc.object())));
    }
}

MetaDataStore::~MetaDataStore()
{

}

void MetaDataStore::setDataSource(DataSourceBase *source)
{
    data_source_.reset(source);
    connect(data_source_.data(),&DataSourceBase::newImage,this,&MetaDataStore::addImage);
    connect(this,&MetaDataStore::start,data_source_.data(),&DataSourceBase::start);
    connect(this,&MetaDataStore::stop,data_source_.data(),&DataSourceBase::stop);
}

DataPtr MetaDataStore::at(int index) const
{
    return data_.at(index);
}

int MetaDataStore::size() const
{
    return data_.size();
}

int MetaDataStore::indexOf(const DataPtr &ptr) const
{
    return data_.indexOf(ptr);
}

bool MetaDataStore::empty() const
{
    return data_.empty();
}

void MetaDataStore::clear()
{
    data_.clear();
}

void MetaDataStore::setProjectDir(const QString &epu_project_dir)
{
    data_source_->setProjectDir(epu_project_dir);
}

void MetaDataStore::setMovieDir(const QString &movie_dir)
{
    data_source_->setMovieDir(movie_dir);
}

void MetaDataStore::saveData(const DataPtr &ptr)
{
    QDir cryoflare_dir(CRYOFLARE_DIRECTORY);
    QString file_name(cryoflare_dir.filePath(ptr->value("short_name").toString()));
    QFile save_file(file_name);
    if (!save_file.open(QIODevice::WriteOnly)) {
        qWarning() << "Couldn't save file: " <<file_name;
        return;
    }
    QJsonDocument save_doc(*ptr.data());
    save_file.write(save_doc.toBinaryData());
}

void MetaDataStore::addImage(const DataPtr &ptr)
{
    QString name=ptr->value("name").toString();
    foreach(DataPtr entry, data_){
        if(name==entry->value("name").toString()){
            //skip already existing entries
            return;
        }
    }
    data_.append(ptr);
    emit newImage(ptr);
}
