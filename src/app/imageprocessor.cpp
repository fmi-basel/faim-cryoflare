//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFlare
//
// Copyright (C) 2017-2018 by the CryoFlare Authors
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
// along with CryoFlare.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------------

#include <iostream>
#include <QApplication>
#include <QDir>
#include <QDateTime>
#include "settings.h"
#include <QtDebug>
#include <QProcess>
#include <processwrapper.h>
#include "imageprocessor.h"
#include "metadatastore.h"


ImageProcessor::ImageProcessor(MetaDataStore &meta_data_store):
    QObject(),
    epu_project_dir_(),
    movie_dir_(),
    cpu_pqueue_(),
    gpu_pqueue_(),
    cpu_processes_(),
    gpu_processes_(),
    root_task_(new Task("General","dummy",DataPtr(new Data))),
    exporters_(),
    current_exporter_(nullptr),
    process_(new QProcess(this)),
    running_state_(false),
    meta_data_store_(meta_data_store)

{
    connect(&meta_data_store_,&MetaDataStore::newImage,[=](const DataPtr& ptr) { createTaskTree( ptr, false); });
    QTimer::singleShot(0, this, SLOT(loadSettings()));
}

ImageProcessor::~ImageProcessor()
{
    foreach (ProcessWrapper* process, cpu_processes_) {
        process->terminate();
    }
    foreach (ProcessWrapper* process, gpu_processes_) {
        process->terminate();
    }
}

void ImageProcessor::startStop(bool start)
{
    if(start){
        Settings settings;
        running_state_=true;
        epu_project_dir_=settings.value("avg_source_dir").toString();
        movie_dir_=settings.value("stack_source_dir").toString();
        meta_data_store_.setProjectDir(epu_project_dir_);
        meta_data_store_.setMovieDir(movie_dir_);
        meta_data_store_.start();
        startTasks();
    }else{
        running_state_=false;
        meta_data_store_.stop();
        foreach(ProcessWrapper* process, cpu_processes_){
            // terminate and re-enqueue task
            if(process->task()!=nullptr){
                cpu_pqueue_.enqueue(process->task());
            }
            if(process->running()){
                process->terminate();
            }
        }
        foreach(ProcessWrapper* process, gpu_processes_){
            // terminate and re-enqueue task
            if(process->task()!=nullptr){
                gpu_pqueue_.enqueue(process->task());
            }
            if(process->running()){
                process->terminate();
            }
        }
    }
}

void ImageProcessor::onTaskFinished(const TaskPtr &task)
{
    if(task->state==0){
        //task finished successfully
        enqueueChildren_(task);
        emit queueCountChanged(cpu_pqueue_.size(),gpu_pqueue_.size());
        task->data->insert(task->taskString(),"FINISHED");
    }else{
        // task aborted
        task->data->insert(task->taskString(),"ERROR");
    }
    startTasks();
    QFile f(QDir::current().relativeFilePath(task->name+"_out.log"));
    if (f.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream( &f );
        stream << task->output << endl;
    }
    QFile ferr(QDir::current().relativeFilePath(task->name+"_error.log"));
    if (ferr.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream( &ferr );
        stream << task->error << endl;
    }
    meta_data_store_.saveData(task->data);
    emit dataChanged(task->data);
}



void ImageProcessor::loadSettings()
{
     emit processesDeleted();
     Settings *settings=new Settings;
     while(!cpu_processes_.empty()){
         cpu_processes_.takeLast()->deleteLater();
     }
     while(!gpu_processes_.empty()){
         gpu_processes_.takeLast()->deleteLater();
     }
     int num_cpu=settings->value("num_cpu",10).toInt();
     int num_gpu=settings->value("num_gpu",2).toInt();
     int timeout=settings->value("timeout",300).toInt();
     QStringList gpu_ids=settings->value("gpu_ids","0").toString().split(",", QString::SkipEmptyParts);
     if(gpu_ids.empty()){
         for(int i=0;i<num_gpu;++i) {
             gpu_ids << QString("%1").arg(i);
         }
     }
     for(int i=0;i<num_cpu;++i) {
         ProcessWrapper* wrapper=new ProcessWrapper(this,timeout,-1);
         emit processCreated(wrapper,-1);
         connect(wrapper,&ProcessWrapper::finished,this,&ImageProcessor::onTaskFinished);
         cpu_processes_.append(wrapper);

     }
     for(int i=0;i<num_gpu;++i) {
         int gpu_id=gpu_ids.at(i%gpu_ids.size()).toInt();
         ProcessWrapper* wrapper=new ProcessWrapper(this,timeout,gpu_id);
         emit processCreated(wrapper,gpu_id);
         connect(wrapper,&ProcessWrapper::finished,this,&ImageProcessor::onTaskFinished);
         gpu_processes_.append(wrapper);
     }

    root_task_->children.clear();
    settings->beginGroup("Tasks");
    loadTask_(settings,root_task_);
    settings->endGroup();
    delete settings;

}

void ImageProcessor::exportImages(const SftpUrl &export_path, const SftpUrl &raw_export_path, const QStringList &image_list, const QStringList &output_keys, const QStringList &raw_keys, const QStringList &shared_keys, bool duplicate_raw)
{
    Settings settings;
    QString export_mode=settings.value("export").toString();
    if(export_mode=="custom2"){
        QString custom_script=settings.value("export_custom_script").toString();
        QFileInfo check_file(custom_script);
        if (!check_file.exists() || !check_file.isFile()) {
	    qInfo() << "Custom export script doesn't exist:" << custom_script;
            return;
        }
        process_->setProcessChannelMode(QProcess::MergedChannels);
        process_->setStandardOutputFile(QDir(QDir::currentPath()).absoluteFilePath("export.log"));
        QStringList arguments;
        arguments << QDir::currentPath();
        process_->start(custom_script,arguments);
        process_->waitForStarted(-1);
        //todo merge logic into meta data store
        foreach(DataPtr ptr, meta_data_store_.images()){
            QStringList keys=ptr->keys();
            keys=keys.filter("_CryoFLARE_TASK_");
            foreach(QString key,keys){
                if(ptr->value(key).toString()!=QString("FINISHED")){
                    continue;
                }
            }
            QString name=ptr->value("name").toString();
            if(image_list.contains(name)){
                QJsonObject raw_files=ptr->value("raw_files").toObject();
                QJsonObject files=ptr->value("files").toObject();
                QJsonObject shared_files=ptr->value("shared_files").toObject();
                QStringList raw_paths;
                foreach(QString key,raw_files.keys()){
                    if(raw_keys.contains(key)){
                        raw_paths.append(QDir::current().relativeFilePath((raw_files.value(key).toString())));
                    }
                }
                process_->write(QString("raw_%1=%2\n").arg(name).arg(raw_paths.join(",")).toLatin1());
                QStringList file_paths;
                foreach(QString key,files.keys()){
                    if(output_keys.contains(key)){
                        file_paths.append(QDir::current().relativeFilePath((files.value(key).toString())));
                    }
                }
                process_->write(QString("%1=%2\n").arg(name).arg(raw_paths.join(",")).toLatin1());
            }
        }
        QStringList shared_list;
        foreach(QString f, meta_data_store_.sharedFiles(image_list,shared_keys)){
            shared_list.append(QDir::current().relativeFilePath((f)));
        }
        process_->write(QString("%1=%2\n").arg("shared").arg(shared_list.join(",")).toLatin1());
        process_->closeWriteChannel();
    }else{
        bool separate_raw_export=export_path!=raw_export_path;
        int num_processes=settings.value("export_num_processes",1).toInt();

        QStringList files;
        QStringList raw_files;
        QStringList abs_raw_files=meta_data_store_.rawFiles(image_list,raw_keys);
        QDir parent_dir=QDir::current();
        parent_dir.cdUp();
        foreach(QString f,abs_raw_files){
            raw_files.append(parent_dir.relativeFilePath(f));
        }
        QStringList abs_output_files=meta_data_store_.outputFiles(image_list,output_keys);
        foreach(QString f,abs_output_files){
            files.append(parent_dir.relativeFilePath(f));
        }
        QStringList files_to_filter;
        QStringList abs_shared_files=meta_data_store_.sharedFiles(image_list,shared_keys);
        foreach(QString f,abs_shared_files){
            if(f.endsWith(".star")){
                files_to_filter.append(parent_dir.relativeFilePath(f));
            }else{
                files.append(parent_dir.relativeFilePath(f));
            }
        }
        if( (!separate_raw_export) || duplicate_raw){
            files.append(raw_files);
        }
        if(!files.empty() || !files_to_filter.empty()){
            ParallelExporter* exporter=new ParallelExporter(parent_dir.path(),export_path,image_list,num_processes);
            exporter->addImages(files_to_filter,true);
            exporter->addImages(files,false);
            exporters_.enqueue(exporter);
        }
        if(separate_raw_export && !raw_files.empty()){
            ParallelExporter* raw_exporter=new ParallelExporter(parent_dir.path(),raw_export_path,image_list,num_processes);
            raw_exporter->addImages(raw_files,false);
            exporters_.enqueue(raw_exporter);
        }
        startNextExport_();
    }

}

void ImageProcessor::cancelExport()
{
    if(current_exporter_){
        current_exporter_->cancel();
        exportFinished_();
    }
}

void ImageProcessor::startTasks()
{
    if (! running_state_){
        return;
    }
    bool count_changed=false;
    foreach (ProcessWrapper* proc, cpu_processes_) {
       if(! proc->running() &&  ! cpu_pqueue_.empty()){
           proc->start(cpu_pqueue_.dequeue());
           count_changed=true;
       }
    }
    foreach (ProcessWrapper* proc, gpu_processes_) {
       if(! proc->running() &&  ! gpu_pqueue_.empty()){
           proc->start(gpu_pqueue_.dequeue());
           count_changed=true;
       }
    }
    if(count_changed){
        emit queueCountChanged(cpu_pqueue_.size(),gpu_pqueue_.size());
    }
}


void ImageProcessor::exportFinished_()
{
    delete current_exporter_;
    current_exporter_=nullptr;
    emit exportFinished();
    startNextExport_();
}

void ImageProcessor::startNextExport_()
{
    if(current_exporter_){
        // don't start new export if one is already running
        return;
    }
    if(!exporters_.empty()){
        current_exporter_=exporters_.dequeue();
        //finished signal needs to go to GUI first, before current exporter is deleted
        connect(current_exporter_,&ParallelExporter::finished,this,&ImageProcessor::exportFinished_);
        connect(current_exporter_,&ParallelExporter::message,this,&ImageProcessor::exportMessage);
        current_exporter_->start();
        emit exportStarted(current_exporter_->destination().toString(QUrl::RemovePassword),current_exporter_->numFiles());
    }else{
    }
}


void ImageProcessor::createTaskTree(DataPtr data, bool force_reprocess)
{
    //qDebug() << "create task tree for: " << data->value("name").toString();
    TaskPtr root_task=root_task_->clone();
    root_task->setData(data,force_reprocess);
    enqueueChildren_(root_task);
    emit queueCountChanged(cpu_pqueue_.size(),gpu_pqueue_.size());
    startTasks();
}

void ImageProcessor::reprocess(const QVector<DataPtr> &images)
{
    foreach(DataPtr ptr,images){
        foreach(ProcessWrapper* wrapper,cpu_processes_){
            if(!wrapper->running()){
                continue;
            }
            if(wrapper->task()->data->value("name").toString()==ptr->value("name").toString()){
                wrapper->terminate();
            }
        }
        foreach(ProcessWrapper* wrapper,gpu_processes_){
            if(!wrapper->running()){
                continue;
            }
            if(wrapper->task()->data->value("name").toString()==ptr->value("name").toString()){
                wrapper->terminate();
            }
        }
        QJsonObject raw_files=ptr->value("raw_files").toObject();
        QJsonObject files=ptr->value("files").toObject();
        foreach(QString key,raw_files.keys()){
            QFile file(raw_files.value(key).toString());
            file.remove();
        }
        foreach(QString key,files.keys()){
            QFile file(files.value(key).toString());
            file.remove();
        }
        ptr->insert("raw_files",QJsonObject());
        ptr->insert("files",QJsonObject());
        ptr->insert("shared_files",QJsonObject());
        createTaskTree(ptr,true);
    }
}


void ImageProcessor::loadTask_(Settings *settings, const TaskPtr &task)
{
    foreach(QString child_name,settings->childGroups()){
        settings->beginGroup(child_name);
        TaskPtr child(new Task(child_name,settings->value("script").toString(), DataPtr(new Data()),settings->value("is_gpu").toBool(),settings->value("is_priority").toBool()));
        task->children.append(child);
        loadTask_(settings,child);
        settings->endGroup();
    }

}

void ImageProcessor::enqueueChildren_(const TaskPtr &task)
{
    for(int i=0;i<task->children.size();++i){
        TaskPtr child=task->children.at(i);
        if(child->data->value(child->taskString()).toString()=="FINISHED"){
            enqueueChildren_(child);
        }else{
            PriorityQueue& stack=child->gpu?gpu_pqueue_:cpu_pqueue_;
            stack.enqueue(child);
        }
    }
}

