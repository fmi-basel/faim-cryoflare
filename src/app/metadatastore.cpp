//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2019 by the CryoFLARE Authors
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3.0 of the License.
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License
// along with CryoFLARE.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------------
#include <QDebug>
#include <QDir>
#include <QTimer>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "settings.h"
#include "metadatastore.h"
#include "flatfolderdatasource.h"
#include "epudatasource.h"
#include "imagetablemodel.h"
#include "filereaders.h"
#include "sftpurl.h"
#include <LimeReport>
#include <QtConcurrent/QtConcurrentMap>
#include <QtConcurrent/QtConcurrentRun>

static const bool BINARY_STORAGE = true;


void PersistenDataWriter::writeData(QJsonObject data, const QString &basename){
    QString filename=QDir(CRYOFLARE_DIRECTORY).filePath(QString(BINARY_STORAGE ? "%1.dat":"%1.json").arg(basename));
    QFile save_file(filename);
    if (!save_file.open(QIODevice::WriteOnly)) {
        qWarning() << "Couldn't save file: " <<filename;
        return;
    }
    QJsonDocument save_doc(data);
    save_file.write(BINARY_STORAGE ? save_doc.toBinaryData() : save_doc.toJson());
}

MetaDataStore::MetaDataStore(TaskConfiguration* task_configuration, QObject *parent):
    QObject(parent),
    task_configuration_(task_configuration),
    data_folder_watcher_(),
    micrographs_(),
    foil_holes_(),
    grid_squares_(),
    worker_(),
    exporters_(),
    current_exporter_(nullptr)
{
    Settings settings;
    data_folder_watcher_=createFolderWatcher_(settings.value("import").toString(),settings.value("import_image_pattern").toString());
    connect(data_folder_watcher_,&DataFolderWatcher::newDataAvailable,this, [=](const ParsedData& parsed_data) {updateData(parsed_data,true);});

     QTimer::singleShot(0,this,&MetaDataStore::readPersistentData_);
     PersistenDataWriter *writer=new PersistenDataWriter;
     writer->moveToThread(&worker_);
     connect(&worker_, &QThread::finished, writer, &QObject::deleteLater);
     connect(this, &MetaDataStore::saveData, writer, &PersistenDataWriter::writeData);
     worker_.start();
}

MetaDataStore::~MetaDataStore()
{
    worker_.quit();
    worker_.wait();
}


Data MetaDataStore::at(int index) const
{
    return micrographs_.value(micrographs_.keys()[index]);
}

Data MetaDataStore::micrograph(const QString &id) const
{
    return micrographs_.value(id);
}

Data MetaDataStore::gridsquare(const QString &id) const
{
    return grid_squares_[id];
}

QMap<QString, Data> MetaDataStore::gridsquares() const
{
    return grid_squares_;
}

QMap<QString, Data> MetaDataStore::foilholes() const
{
    return foil_holes_;
}

Data MetaDataStore::gridsquareAt(int index) const
{
    return grid_squares_.value(grid_squares_.keys()[index]);
}

Data MetaDataStore::foilhole(const QString &id) const
{
    return foil_holes_[id];
}

bool MetaDataStore::hasGridsquare(const QString &id) const
{
    return grid_squares_.contains(id);
}

bool MetaDataStore::hasFoilhole(const QString &id) const
{
    return foil_holes_.contains(id);
}

int MetaDataStore::gridsquareCount() const
{
    return grid_squares_.size();
}

int MetaDataStore::foilholeCount() const
{
    return foil_holes_.size();
}

int MetaDataStore::size() const
{
    return micrographs_.size();
}

int MetaDataStore::indexOf(const QString& id) const
{
    return micrographs_.keys().indexOf(id);
}

bool MetaDataStore::empty() const
{
    return micrographs_.empty();
}

void MetaDataStore::clear()
{
    micrographs_.clear();
}


QSet<QString> MetaDataStore::rawKeys() const
{
    QSet<QString> result;
    foreach(Data data, micrographs_){
        QJsonObject raw_files=data.value("raw_files").toObject();
        result.unite(QSet<QString>::fromList(raw_files.keys()));
    }
    return result;
}

QSet<QString> MetaDataStore::outputKeys() const
{
    QSet<QString> result;
    foreach(Data data, micrographs_){
        QJsonObject files=data.value("files").toObject();
        result.unite(QSet<QString>::fromList(files.keys()));
    }
    return result;
}

QSet<QString> MetaDataStore::sharedKeys() const
{
    QSet<QString> result;
    foreach(Data data, micrographs_){
        QJsonObject shared_files=data.value("shared_files").toObject();
        result.unite(QSet<QString>::fromList(shared_files.keys()));
    }
    return result;
}

QSet<QString> MetaDataStore::sharedRawKeys() const
{
    QSet<QString> result;
    foreach(Data data, micrographs_){
        QJsonObject shared_raw_files=data.value("shared_raw_files").toObject();
        result.unite(QSet<QString>::fromList(shared_raw_files.keys()));
    }
    return result;
}

void MetaDataStore::saveMicrographData_(const QString &id)
{
    emit saveData(micrographs_.value(id),id);
    emit micrographUpdated(id);
}

void MetaDataStore::saveFoilholeData_(const QString &id)
{
    emit saveData(foil_holes_.value(id),QString("%1/%2").arg(CRYOFLARE_FOILHOLES_DIRECTORY).arg(id));
    emit foilholeUpdated(id);
}

void MetaDataStore::saveGridsquareData_(const QString &id)
{
    emit saveData(grid_squares_.value(id),QString("%1/%2").arg(CRYOFLARE_GRIDSQUARES_DIRECTORY).arg(id));
    emit gridsquareUpdated(id);
}

void MetaDataStore::updateData(const ParsedData &data, bool save)
{
    auto update_item = [] (MetaDataStore *store, QMap<QString,Data> &container, const Data& d, void (MetaDataStore::*sig)(const QString&)){
        QString id=d.id();
        if(container.contains(id)){
            Data merged=container.value(id);
            merged.update(d);
            container.insert(id,merged);
        }else{
            container.insert(id,d);
            emit (store->*sig)(id);
        }
    };

    auto update_children = [] (MetaDataStore *store, QMap<QString,Data> &container, const Data& d, void (MetaDataStore::*sig)(const QString&),void (MetaDataStore::*save_method)(const QString&),bool save){
        foreach(QString child,d.children()){
            Data child_data;
            if(! container.contains(child)){
                container.insert(child,Data());
                emit (store->*sig)(child);
            }
            child_data=container.value(child);
            if(child_data.parent()==""){
                child_data.setParent(d.id());
                container.insert(child,child_data);
                if(save){
                    (store->*save_method)(child);
                }
            }
        }
    };
    auto update_parent = [] (MetaDataStore *store, QMap<QString,Data> &container, const Data& d, void (MetaDataStore::*sig)(const QString&),void (MetaDataStore::*save_method)(const QString&),bool save){
        QString parent_id=d.parent();
        Data parent_data;
        if(! container.contains(parent_id)){
            container.insert(parent_id,Data());
            emit (store->*sig)(parent_id);
        }
        parent_data=container.value(parent_id);
        if(! parent_data.children().contains(d.id())){
            parent_data.addChild(d.id());
            container.insert(parent_id,parent_data);
            if(save){
                (store->*save_method)(parent_id);
            }
        }
    };
    foreach(Data d, data.grid_squares){
        update_item(this,grid_squares_,d,&MetaDataStore::newGridsquare);
        if(save){
            saveGridsquareData_(d.id());
        }
        update_children(this,foil_holes_,grid_squares_.value(d.id()),&MetaDataStore::newFoilhole,&MetaDataStore::saveFoilholeData_,save);
    }
    foreach(Data d, data.foil_holes){
        update_item(this,foil_holes_,d,&MetaDataStore::newFoilhole);
        if(save){
            saveFoilholeData_(d.id());
        }
        update_children(this,micrographs_,foil_holes_.value(d.id()),&MetaDataStore::newMicrograph,&MetaDataStore::saveMicrographData_,save);
        update_parent(this,grid_squares_,foil_holes_.value(d.id()),&MetaDataStore::newGridsquare,&MetaDataStore::saveGridsquareData_,save);
    }
    foreach(Data d, data.micrographs){
        update_item(this,micrographs_,d,&MetaDataStore::newMicrograph);
        Data mic_data=micrographs_.value(d.id());
        if(!mic_data.contains("export")){
            mic_data.insert("export","true");
        }
        if(!mic_data.contains("raw_files")){
            mic_data.insert("raw_files",QJsonObject());
        }
        if(!mic_data.contains("files")){
            mic_data.insert("files",QJsonObject());
        }
        if(!mic_data.contains("shared_files")){
            mic_data.insert("shared_files",QJsonObject());
        }
        if(!mic_data.contains("shared_raw_files")){
            mic_data.insert("shared_raw_files",QJsonObject());
        }
        micrographs_.insert(d.id(),mic_data);
        if(save){
            saveMicrographData_(d.id());
        }
        update_parent(this,foil_holes_,mic_data,&MetaDataStore::newFoilhole,&MetaDataStore::saveFoilholeData_,save);
    }
}

void MetaDataStore::start(const QString &project_dir, const QString &movie_dir)
{
    data_folder_watcher_->setProjectDir(project_dir);
    data_folder_watcher_->setMovieDir(movie_dir);
    data_folder_watcher_->start();
}

void MetaDataStore::stop()
{
    data_folder_watcher_->stop();
}

void MetaDataStore::readPersistentData_()
{
    ParsedData parsed_data;
    struct reader
    {
        reader(bool binary,const QString& dirpath):binary_(binary),dirpath_(dirpath){}
        typedef Data result_type;
        Data operator()(const QString& path)
        {
            QFile load_file(QDir(dirpath_).absoluteFilePath(path));
            if(!load_file.open(QIODevice::ReadOnly)){
                qWarning() << "Couldn't open file: " << QDir(dirpath_).absoluteFilePath(path);
                return Data();
            }
            QByteArray byte_data=load_file.readAll();
            QJsonDocument load_doc=binary_ ? QJsonDocument::fromBinaryData(byte_data) : QJsonDocument::fromJson(byte_data);
            return load_doc.object();
        }

        bool binary_;
        QString dirpath_;
    };
    QString path=CRYOFLARE_DIRECTORY;
    parsed_data.micrographs=QtConcurrent::blockingMapped<QList<Data> >(QDir(path).entryList(BINARY_STORAGE? QStringList("*.dat") :  QStringList("*.json"),QDir::Files,QDir::Name),reader(BINARY_STORAGE,path));
    path=QString("%1/%2").arg(CRYOFLARE_DIRECTORY).arg(CRYOFLARE_FOILHOLES_DIRECTORY);
    parsed_data.foil_holes=QtConcurrent::blockingMapped<QList<Data> >(QDir(path).entryList(BINARY_STORAGE? QStringList("*.dat") :  QStringList("*.json"),QDir::Files,QDir::Name),reader(BINARY_STORAGE,path));
    path=QString("%1/%2").arg(CRYOFLARE_DIRECTORY).arg(CRYOFLARE_GRIDSQUARES_DIRECTORY);
    parsed_data.grid_squares=QtConcurrent::blockingMapped<QList<Data> >(QDir(path).entryList(BINARY_STORAGE? QStringList("*.dat") :  QStringList("*.json"),QDir::Files,QDir::Name),reader(BINARY_STORAGE,path));
    updateData(parsed_data,false);
}


QList<QString> MetaDataStore::micrographIDs() const
{
    return micrographs_.keys();
}

QList<QString> MetaDataStore::selectedMicrographIDs() const
{
    QList<QString> result;
    foreach(QString id,micrographIDs()){
        QString export_string=micrographs_.value(id).value("export").toString("true");
        if(export_string.compare("true", Qt::CaseInsensitive) == 0 || export_string==QString("1")){
            result.append(id);
        }
    }
    return result;
}

void MetaDataStore::setMicrographExport(const QString &id, bool export_flag)
{
    micrographs_[id].insert("export",export_flag?"true":"false");
    micrographs_[id].setTimestamp(QDateTime::currentDateTime());
    saveMicrographData_(id);
}

void MetaDataStore::updateMicrograph(const QString &id, const QMap<QString,QString>& new_data,  const QMap<QString, QString> &raw_files,  const QMap<QString, QString> &files,  const QMap<QString, QString> &shared_files,const QMap<QString,QString>& shared_raw_files)
{
    Data& data=micrographs_[id];
    foreach(QString key,new_data.keys()){
        data.insert(key,new_data[key]);
    }
    QVariantMap old_raw_files;
    if(data.contains("raw_files")){
        old_raw_files=data.value("raw_files").toObject().toVariantMap();
    }
    foreach(QString key,raw_files.keys()){
        old_raw_files.insert(key,raw_files.value(key));
    }
    QVariantMap old_files;
    if(data.contains("files")){
        old_files=data.value("files").toObject().toVariantMap();
    }
    foreach(QString key,files.keys()){
        old_files.insert(key,files.value(key));
    }
    QVariantMap old_shared_files;
    if(data.contains("shared_files")){
        old_shared_files=data.value("shared_files").toObject().toVariantMap();
    }
    foreach(QString key,shared_files.keys()){
        old_shared_files.insert(key,shared_files.value(key));
    }
    QVariantMap old_shared_raw_files;
    if(data.contains("shared_raw_files")){
        old_shared_raw_files=data.value("shared_raw_files").toObject().toVariantMap();
    }
    foreach(QString key,shared_raw_files.keys()){
        old_shared_raw_files.insert(key,shared_raw_files.value(key));
    }
    data.insert("raw_files",QJsonObject::fromVariantMap(old_raw_files));
    data.insert("files",QJsonObject::fromVariantMap(old_files));
    data.insert("shared_files",QJsonObject::fromVariantMap(old_shared_files));
    data.insert("shared_raw_files",QJsonObject::fromVariantMap(old_shared_raw_files));
    data.setTimestamp(QDateTime::currentDateTime());
    saveMicrographData_(id);
}

void MetaDataStore::removeMicrographResults(const QString &id, const TaskDefinitionPtr &definition)
{
    Data& data=micrographs_[id];
    QJsonObject raw_files=data.value("raw_files").toObject();
    QJsonObject files=data.value("files").toObject();
    //foreach(QString key,raw_files.keys()){
    //    QFile file(raw_files.value(key).toString());
    //    file.remove();
    //}
    foreach(QString key,files.keys()){
        QFile file(files.value(key).toString());
        file.remove();
    }
    data.insert("raw_files",QJsonObject());
    data.insert("files",QJsonObject());
    data.insert("shared_files",QJsonObject());
    data.insert("shared_raw_files",QJsonObject());
    QList<TaskDefinitionPtr> definition_list;
    definition_list.append(definition);
    while(! definition_list.empty()){
        TaskDefinitionPtr current=definition_list.takeFirst();
        data.insert(current->taskString(),"REMOVED");
        definition_list.append(current->children);
    }
    saveMicrographData_(id);
}

void MetaDataStore::createReport(const QString &file_name, const QString &type)
{
    QScopedPointer<ImageTableModel> model(new ImageTableModel(this,task_configuration_));
    if(type=="PDF Report"){
        LimeReport::ReportEngine report;
        report.dataManager()->addModel("Images",model.data(),false);
        Settings settings;
        if(QFileInfo::exists(settings.value("report_template").toString())){
          report.loadFromFile(settings.value("report_template").toString());
        }
        report.printToPDF(file_name);
    }else{
        bool filtered=type.endsWith("filtered");
        QFile file(file_name);
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
            return;
        }
        QTextStream out(&file);
        QList<InputOutputVariable> result_labels=task_configuration_->resultLabels();
        if(type=="CSV" || type=="CSV filtered"){
            QStringList header;
            foreach(InputOutputVariable v, result_labels){
                header.append(v.key);
            }
            out << header.join(",") << "\n";
            foreach(Data data,micrographs_){
                QString export_val=data.value("export").toString("true");
                if(filtered && export_val.compare("true", Qt::CaseInsensitive)!=0 && export_val!=QString("1") ){
                    continue;
                }
                QStringList row;
                foreach(InputOutputVariable v, result_labels){
                    row.append(data.value(v.label).toString());
                }
                out << row.join(",") << "\n";
            }
        }else if(type=="JSON" || type=="JSON filtered"){
            QJsonObject root_object;
            foreach(Data data,micrographs_){
                QJsonObject child_object;
                QString export_val=data.value("export").toString("true");
                if(filtered && export_val.compare("true", Qt::CaseInsensitive)!=0 && export_val!=QString("1") ){
                    continue;
                }
                QStringList row;
                foreach(InputOutputVariable v, result_labels){
                    child_object.insert(v.label,data.value(v.label));
                }
                root_object.insert(data.value("id").toString(),child_object);
            }
            QJsonDocument doc(root_object);
            file.write(doc.toJson());
        }
    }
}
void MetaDataStore::exportMicrographs(const SftpUrl &export_path, const SftpUrl &raw_export_path, const QStringList &output_keys, const QStringList &raw_keys, const QStringList &shared_keys, const QStringList& shared_raw_keys, bool duplicate_raw)
{
    bool separate_raw_export=export_path!=raw_export_path;
    Settings settings;
    int num_processes=settings.value("export_num_processes",1).toInt();
    QStringList files;
    QStringList raw_files;
    QSet<QString> filtered_shared_files;
    QSet<QString> unfiltered_shared_files;
    QSet<QString> filtered_shared_raw_files;
    QSet<QString> unfiltered_shared_raw_files;
    QDir parent_dir=QDir::current();
    parent_dir.cdUp();
    foreach(QString id, selectedMicrographIDs()){
        Data& data=micrographs_[id];
        QJsonObject file_object=data.value("files").toObject();
        foreach(QString key,file_object.keys()){
            if(output_keys.contains(key)){
                files.append(parent_dir.relativeFilePath(QDir::current().absoluteFilePath(file_object.value(key).toString())));
            }
        }
        file_object=data.value("raw_files").toObject();
        foreach(QString key,file_object.keys()){
            if(raw_keys.contains(key)){
                raw_files.append(parent_dir.relativeFilePath(QDir::current().absoluteFilePath(file_object.value(key).toString())));
            }
        }
        file_object=data.value("shared_files").toObject();
        foreach(QString key,file_object.keys()){
            if(shared_keys.contains(key)){
                QString f=file_object.value(key).toString();
                if(f.endsWith(".star")){
                    filtered_shared_files.insert(parent_dir.relativeFilePath(QDir::current().absoluteFilePath(f)));
                }else{
                    unfiltered_shared_files.insert(parent_dir.relativeFilePath(QDir::current().absoluteFilePath(f)));
                }
            }
        }
        file_object=data.value("shared_raw_files").toObject();
        foreach(QString key,file_object.keys()){
            if(shared_raw_keys.contains(key)){
                QString f=file_object.value(key).toString();
                if(f.endsWith(".star")){
                    filtered_shared_raw_files.insert(parent_dir.relativeFilePath(QDir::current().absoluteFilePath(f)));
                }else{
                    unfiltered_shared_raw_files.insert(parent_dir.relativeFilePath(QDir::current().absoluteFilePath(f)));
                }
            }
        }
    }
    if( (!separate_raw_export) || duplicate_raw){
        files.append(raw_files);
        filtered_shared_files.unite(filtered_shared_raw_files);
        unfiltered_shared_files.unite(unfiltered_shared_raw_files);
    }
    files.append(unfiltered_shared_files.toList());
    if(!files.empty() || !filtered_shared_files.empty()){
        ParallelExporter* exporter=new ParallelExporter(parent_dir.path(),export_path,selectedMicrographIDs(),num_processes);
        exporter->addImages(filtered_shared_files.toList(),true);
        exporter->addImages(files,false);
        exporters_.enqueue(exporter);
    }
    raw_files.append(unfiltered_shared_raw_files.toList());
    if(separate_raw_export && ! (raw_files.empty() && filtered_shared_raw_files.empty() )){
        ParallelExporter* raw_exporter=new ParallelExporter(parent_dir.path(),raw_export_path,selectedMicrographIDs(),num_processes);
        raw_exporter->addImages(filtered_shared_raw_files.toList(),true);
        raw_exporter->addImages(raw_files,false);
        exporters_.enqueue(raw_exporter);
    }
    startNextExport_();

}

QString MetaDataStore::value(const QString &id, QString key) const
{
    if(! micrographs_.contains(id)){
        return QString();
    }
    if(key.startsWith("hole_")){
        Data mic_data=micrographs_.value(id);
        QString pid=mic_data.parent();
        Data hole_data=foil_holes_.value(micrographs_.value(id).parent());
        key.remove(0,5);
        if(hole_data.contains(key)){
            return hole_data.value(key).toString();
        }
    }else if(key.startsWith("square_")){
        Data square_data=grid_squares_.value(foil_holes_.value(micrographs_.value(id).parent()).parent());
        key.remove(0,7);
        if(square_data.contains(key)){
            return square_data.value(key).toString();
        }
    }else{
        if(micrographs_.value(id).contains(key)){
            return micrographs_.value(id).value(key).toString();
        }
    }
    return QString();
}

void MetaDataStore::startNextExport_()
{
    if(current_exporter_){
        // don't start new export if one is already running
        return;
    }
    if(!exporters_.empty()){
        current_exporter_=exporters_.dequeue();
        //finished signal needs to go to GUI first, before current exporter is deleted
        connect(current_exporter_,&ParallelExporter::finished,this,&MetaDataStore::exportFinished_);
        current_exporter_->start();
    }else{
    }
}

DataFolderWatcher * MetaDataStore::createFolderWatcher_(const QString &mode, const QString &pattern)
{
    if(mode=="EPU"){
        return createEPUFolderWatcher(this);
    }else if(mode=="flat_EPU"){
        return createFlatEPUFolderWatcher(this);
    }else if(mode=="json"){
        return createFlatImageFolderWatcher(pattern,this);
    }else{
        return createEPUFolderWatcher(this);
    }
}



void MetaDataStore::exportFinished_()
{
    delete current_exporter_;
    current_exporter_=nullptr;
    startNextExport_();
}
