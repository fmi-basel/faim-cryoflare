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
#include "datasourcebase.h"
#include "epudatasource.h"
#include "imagetablemodel.h"
#include <LimeReport>

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
    micrographs_(),
    foil_holes_(),
    grid_squares_(),
    worker_(),
    exporters_(),
    current_exporter_(nullptr)
{
     QTimer::singleShot(0,this,&MetaDataStore::readPersistenData);
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

void MetaDataStore::addMicrograph( Data data)
{
    if(data.contains("id")){
        QString id=data.value("id").toString();
        if(micrographs_.contains(id)){
            //skip already existing entries
            return;
        }
        if(!data.contains("export")){
            data.insert("export","true");
        }
        if(!data.contains("raw_files")){
            data.insert("raw_files",QJsonObject());
        }
        if(!data.contains("files")){
            data.insert("files",QJsonObject());
        }
        if(!data.contains("shared_files")){
            data.insert("shared_files",QJsonObject());
        }
        micrographs_.insert(id,data);

        saveMicrographData_(id);
        if(data.contains("hole_id") && hasFoilhole(data.value("hole_id").toString())){
            Data hole_data=foilhole(data.value("hole_id").toString());
            QSet<QString> micrograph_ids;
            if(hole_data.contains("micrograph_ids")){
                foreach(QJsonValue jv,hole_data.value("micrograph_ids").toArray()){
                    micrograph_ids.insert(jv.toString());
                }
            }
            micrograph_ids.insert(id);
            QJsonArray micrograph_ids_ja;
            foreach(QString i,micrograph_ids){
                micrograph_ids_ja.append(i);
            }
            hole_data.insert("micrograph_ids",micrograph_ids_ja);
            updateFoilhole(hole_data);
            saveFoilholeData_(data.value("hole_id").toString());
        }
        emit newMicrograph(id);
    }
}

void MetaDataStore::addFoilhole(const Data &data)
{
    if(data.contains("id")){
        QString id=data.value("id").toString();
        if(!foil_holes_.contains(id)){
            foil_holes_.insert(id,data);
            saveFoilholeData_(id);
            if(data.contains("square_id") && hasGridsquare(data.value("square_id").toString())){
                Data square_data=gridsquare(data.value("square_id").toString());
                QSet<QString> hole_ids;
                if(square_data.contains("hole_ids")){
                    foreach(QJsonValue jv,square_data.value("hole_ids").toArray()){
                        hole_ids.insert(jv.toString());
                    }
                }
                hole_ids.insert(id);
                QJsonArray hole_ids_ja;
                foreach(QString i,hole_ids){
                    hole_ids_ja.append(i);
                }
                square_data.insert("hole_ids",hole_ids_ja);
                updateGridsquare(square_data);
            }
            emit newFoilhole(id);
        }
    }
}

void MetaDataStore::addGridsquare(const Data &data)
{
    if(data.contains("id")){
        QString id=data.value("id").toString();
        if(!grid_squares_.contains(id)){
            grid_squares_.insert(id,data);
            saveGridsquareData_(id);
            emit newGridsquare(id);
        }
    }
}


void MetaDataStore::updateFoilhole(const Data &data)
{
    if(data.contains("id")){
        QString id=data.value("id").toString();
        if(foil_holes_.contains(id)){
            foil_holes_.insert(id,data);
            saveFoilholeData_(id);
        }
    }
}

void MetaDataStore::updateGridsquare(const Data &data)
{
    if(data.contains("id")){
        QString id=data.value("id").toString();
        if(grid_squares_.contains(id)){
            grid_squares_.insert(id,data);
            saveGridsquareData_(id);
        }
    }
}

void MetaDataStore::readPersistenData()
{
    readPersistentDataHelper_(CRYOFLARE_DIRECTORY,micrographs_,&MetaDataStore::newMicrograph);
    readPersistentDataHelper_(QString("%1/%2").arg(CRYOFLARE_DIRECTORY).arg(CRYOFLARE_GRIDSQUARES_DIRECTORY),grid_squares_,&MetaDataStore::newGridsquare);
    readPersistentDataHelper_(QString("%1/%2").arg(CRYOFLARE_DIRECTORY).arg(CRYOFLARE_FOILHOLES_DIRECTORY),foil_holes_,&MetaDataStore::newFoilhole);
    foreach(QString id, grid_squares_.keys()){
        QSet<QString> hole_ids;
        Data square_data=grid_squares_.value(id);
        if(square_data.contains("hole_ids")){
            foreach(QJsonValue jv,square_data.value("hole_ids").toArray()){
                hole_ids.insert(jv.toString());
            }
        }
        QJsonArray hole_ids_ja;
        foreach(QString i,hole_ids){
            hole_ids_ja.append(i);
        }
        square_data.insert("hole_ids",hole_ids_ja);
        updateGridsquare(square_data);
        saveGridsquareData_(id);
    }
    foreach(QString id, foil_holes_.keys()){
        Data hole_data=foil_holes_.value(id);
        QSet<QString> micrograph_ids;
        if(hole_data.contains("micrograph_ids")){
            foreach(QJsonValue jv,hole_data.value("micrograph_ids").toArray()){
                micrograph_ids.insert(jv.toString());
            }
        }
        QJsonArray micrograph_ids_ja;
        foreach(QString i,micrograph_ids){
            micrograph_ids_ja.append(i);
        }
        hole_data.insert("micrograph_ids",micrograph_ids_ja);
        updateFoilhole(hole_data);
        saveFoilholeData_(id);

    }
}


void MetaDataStore::readPersistentDataHelper_(const QString &path, QMap<QString,Data> & storage, void(MetaDataStore::*sig)(const QString&))
{
    QDir persistent_dir(path);
    foreach(QString file_entry, persistent_dir.entryList(BINARY_STORAGE? QStringList("*.dat") :  QStringList("*.json"),QDir::Files,QDir::Name)){
        QFile load_file(persistent_dir.filePath(file_entry));
        if(!load_file.open(QIODevice::ReadOnly)){
            qWarning() << "Couldn't open file: " << persistent_dir.filePath(file_entry);
            continue;
        }
        QByteArray byte_data=load_file.readAll();
        QJsonDocument load_doc=BINARY_STORAGE ? QJsonDocument::fromBinaryData(byte_data) : QJsonDocument::fromJson(byte_data);
        Data data=load_doc.object();
        QString id=data.value("id").toString();
        storage.insert(id,data);
        emit (this->*sig)(id);
    }

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
    saveMicrographData_(id);
}

void MetaDataStore::updateMicrograph(const QString &id, const QMap<QString,QString>& new_data,  const QMap<QString, QString> &raw_files,  const QMap<QString, QString> &files,  const QMap<QString, QString> &shared_files)
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
    data.insert("raw_files",QJsonObject::fromVariantMap(old_raw_files));
    data.insert("files",QJsonObject::fromVariantMap(old_files));
    data.insert("shared_files",QJsonObject::fromVariantMap(old_shared_files));
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
                header.append(v.label);
            }
            out << header.join(",") << "\n";
            foreach(Data data,micrographs_){
                QString export_val=data.value("export").toString("true");
                if(filtered && export_val.compare("true", Qt::CaseInsensitive)!=0 && export_val!=QString("1") ){
                    continue;
                }
                QStringList row;
                foreach(InputOutputVariable v, result_labels){
                    row.append(data.value(v.key).toString());
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
                    child_object.insert(v.key,data.value(v.key));
                }
                root_object.insert(data.value("id").toString(),child_object);
            }
            QJsonDocument doc(root_object);
            file.write(doc.toJson());
        }
    }
}
void MetaDataStore::exportMicrographs(const SftpUrl &export_path, const SftpUrl &raw_export_path, const QStringList &output_keys, const QStringList &raw_keys, const QStringList &shared_keys, bool duplicate_raw)
{
    bool separate_raw_export=export_path!=raw_export_path;
    Settings settings;
    int num_processes=settings.value("export_num_processes",1).toInt();
    QStringList files;
    QStringList raw_files;
    QSet<QString> filtered_shared_files;
    QSet<QString> unfiltered_shared_files;
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
    }
    if( (!separate_raw_export) || duplicate_raw){
        files.append(raw_files);
    }
    files.append(unfiltered_shared_files.toList());
    if(!files.empty() || !filtered_shared_files.empty()){
        ParallelExporter* exporter=new ParallelExporter(parent_dir.path(),export_path,selectedMicrographIDs(),num_processes);
        exporter->addImages(filtered_shared_files.toList(),true);
        exporter->addImages(files,false);
        exporters_.enqueue(exporter);
    }
    if(separate_raw_export && !raw_files.empty()){
        ParallelExporter* raw_exporter=new ParallelExporter(parent_dir.path(),raw_export_path,selectedMicrographIDs(),num_processes);
        raw_exporter->addImages(raw_files,false);
        exporters_.enqueue(raw_exporter);
    }
    startNextExport_();

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



void MetaDataStore::exportFinished_()
{
    delete current_exporter_;
    current_exporter_=nullptr;
    startNextExport_();
}
