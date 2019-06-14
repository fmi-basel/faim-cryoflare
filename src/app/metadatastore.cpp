#include <QDebug>
#include <QDir>
#include <QTimer>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include "settings.h"
#include "metadatastore.h"
#include "datasourcebase.h"
#include "epudatasource.h"

MetaDataStore::MetaDataStore(QObject *parent):
    QObject(parent),
    data_source_(nullptr),
    data_()
{
     QTimer::singleShot(0,this,&MetaDataStore::readPersistenData);
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

QStringList MetaDataStore::rawFiles(const QStringList& image_list, const QStringList &key_list, bool finished_only) const
{
    QStringList result;
    foreach(DataPtr ptr, data_){
        if(finished_only){
            QStringList keys=ptr->keys();
            keys=keys.filter("_CryoFLARE_TASK_");
            foreach(QString key,keys){
                if(ptr->value(key).toString()!=QString("FINISHED")){
                    continue;
                }
            }
        }
        if(image_list.contains(ptr->value("short_name").toString())){
            QJsonObject raw_files=ptr->value("raw_files").toObject();
            foreach(QString key,raw_files.keys()){
                if(key_list.contains(key)){
                    result.append(raw_files.value(key).toString());
                }
            }
        }
    }
    return result;
}

QStringList MetaDataStore::outputFiles(const QStringList& image_list, const QStringList& key_list, bool finished_only) const
{
    QStringList result;
    foreach(DataPtr ptr, data_){
        if(finished_only){
            QStringList keys=ptr->keys();
            keys=keys.filter("_CryoFLARE_TASK_");
            foreach(QString key,keys){
                if(ptr->value(key).toString()!=QString("FINISHED")){
                    continue;
                }
            }
        }
        if(image_list.contains(ptr->value("short_name").toString())){
            QJsonObject files=ptr->value("files").toObject();
            foreach(QString key,files.keys()){
                if(key_list.contains(key)){
                    result.append(files.value(key).toString());
                }
            }
        }
    }
    return result;
}

QStringList MetaDataStore::sharedFiles(const QStringList& image_list, const QStringList& key_list, bool finished_only) const
{
    QSet<QString> result;
    foreach(DataPtr ptr, data_){
        if(finished_only){
            QStringList keys=ptr->keys();
            keys=keys.filter("_CryoFLARE_TASK_");
            foreach(QString key,keys){
                if(ptr->value(key).toString()!=QString("FINISHED")){
                    continue;
                }
            }
        }
        if(image_list.contains(ptr->value("short_name").toString())){
            QJsonObject shared_files=ptr->value("shared_files").toObject();
            foreach(QString key,shared_files.keys()){
                if(key_list.contains(key)){
                    result.insert(shared_files.value(key).toString());
                }
            }
        }
    }
    return result.toList();
}

QSet<QString> MetaDataStore::rawKeys() const
{
    QSet<QString> result;
    foreach(DataPtr ptr, data_){
        QJsonObject raw_files=ptr->value("raw_files").toObject();
        result.unite(QSet<QString>::fromList(raw_files.keys()));
    }
    return result;
}

QSet<QString> MetaDataStore::outputKeys() const
{
    QSet<QString> result;
    foreach(DataPtr ptr, data_){
        QJsonObject files=ptr->value("files").toObject();
        result.unite(QSet<QString>::fromList(files.keys()));
    }
    return result;
}

QSet<QString> MetaDataStore::sharedKeys() const
{
    QSet<QString> result;
    foreach(DataPtr ptr, data_){
        QJsonObject shared_files=ptr->value("shared_files").toObject();
        result.unite(QSet<QString>::fromList(shared_files.keys()));
    }
    return result;
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
    QString file_name(QString("%1.dat").arg(cryoflare_dir.filePath(ptr->value("short_name").toString())));
    QFile save_file(file_name);
    if (!save_file.open(QIODevice::WriteOnly)) {
        qWarning() << "Couldn't save file: " <<file_name;
        return;
    }
    QJsonDocument save_doc(*ptr.data());
    save_file.write(save_doc.toBinaryData());
    // save in json text format for debugging
    //QString txt_file_name(QString("%1.json").arg(cryoflare_dir.filePath(ptr->value("short_name").toString())));
    //QFile txt_save_file(txt_file_name);
    //if (!txt_save_file.open(QIODevice::WriteOnly)) {
    //    qWarning() << "Couldn't save file: " <<txt_file_name;
    //    return;
    //}
    //txt_save_file.write(save_doc.toJson());
}

void MetaDataStore::addImage(const DataPtr &ptr)
{
    QString name=ptr->value("name").toString();
    if(!ptr->contains("export")){
        ptr->insert("export","true");
    }
    foreach(DataPtr entry, data_){
        if(name==entry->value("name").toString()){
            //skip already existing entries
            return;
        }
    }
    //ptr->insert("json_metadata",QString::fromLatin1(QJsonDocument(*ptr).toJson()));
    if(!ptr->contains("raw_files")){
        ptr->insert("raw_files",QJsonObject());
    }
    if(!ptr->contains("files")){
        ptr->insert("files",QJsonObject());
    }
    if(!ptr->contains("shared_files")){
        ptr->insert("shared_files",QJsonObject());
    }
    data_.append(ptr);
    saveData(ptr);
    emit newImage(ptr);
}

void MetaDataStore::readPersistenData()
{
    QDir cryoflare_dir(CRYOFLARE_DIRECTORY);
    foreach(QString file_entry, cryoflare_dir.entryList(QStringList("*.dat"),QDir::Files,QDir::Name)){
        QFile load_file(cryoflare_dir.filePath(file_entry));
        if(!load_file.open(QIODevice::ReadOnly)){
            qWarning() << "Couldn't open file: " << cryoflare_dir.filePath(file_entry);
            continue;
        }
        QJsonDocument load_doc=QJsonDocument::fromBinaryData(load_file.readAll());
        addImage(DataPtr(new Data(load_doc.object())));
    }
}

QVector<DataPtr> MetaDataStore::images() const
{
    return data_;
}

QVector<DataPtr> MetaDataStore::selected() const
{
    QVector<DataPtr> result;
    foreach(DataPtr ptr, data_){
        QString selected=ptr->value("export").toString("true");
        if(selected.compare("true", Qt::CaseInsensitive) == 0 || selected==QString("1")){
            result.append(ptr);
        }
    }
    return result;
}
