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


ImageProcessor::ImageProcessor():
    QObject(),
    epu_project_dir_(),
    movie_dir_(),
    cpu_task_stack_(),
    gpu_task_stack_(),
    cpu_processes_(),
    gpu_processes_(),
    root_task_(new Task("General","dummy",DataPtr(new Data))),
    raw_files_(),
    output_files_(),
    shared_output_files_(),
    exporters_(),
    current_exporter_(nullptr),
    process_(new QProcess(this)),
    running_state_(false)

{
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
        raw_files_.clear();
        output_files_.clear();
        shared_output_files_.clear();
        Settings settings;
        running_state_=true;
        epu_project_dir_=settings.value("avg_source_dir").toString();
        movie_dir_=settings.value("stack_source_dir").toString();
        watcher_->addPath(epu_project_dir_);
        onDirChange(epu_project_dir_);
        startTasks();
    }else{
        running_state_=false;
        watcher_->removePath(epu_project_dir_);
        cpu_task_stack_.clear();
        gpu_task_stack_.clear();
        foreach(ProcessWrapper* process, cpu_processes_){
            process->terminate();
        }
        foreach(ProcessWrapper* process, gpu_processes_){
            process->terminate();
        }
    }
}

void ImageProcessor::onTaskFinished(const TaskPtr &task, bool gpu)
{
    if(! output_files_.contains(task->data->value("short_name"))){
        output_files_[task->data->value("short_name")]=QMap<QString,QString>();
    }
    foreach(QString key, task->output_files.keys()){
        output_files_[task->data->value("short_name")].insert(key,QDir::current().relativeFilePath(task->output_files.value(key)));
    }
    if(! raw_files_.contains(task->data->value("short_name"))){
        raw_files_[task->data->value("short_name")]=QMap<QString,QString>();
    }
    foreach(QString key, task->raw_files.keys()){
        raw_files_[task->data->value("short_name")].insert(key,QDir::current().relativeFilePath(task->raw_files.value(key)));
    }
    foreach(QString key, task->shared_output_files.keys()){
        shared_output_files_.insert(key,QDir::current().relativeFilePath( task->shared_output_files.value(key)));
    }
    foreach(TaskPtr child,task->children){
        QStack<TaskPtr>& stack=child->gpu?gpu_task_stack_:cpu_task_stack_;
        stack.append(child);
    }
    emit queueCountChanged(cpu_task_stack_.size(),gpu_task_stack_.size());
    startTasks();
    QFile f(QDir::current().relativeFilePath(task->name+"_out.log"));
    if (f.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream( &f );
        stream << task->output << endl;
    }
    QFile ferr(QDir::current().relativeFilePath(task->name+"_error.log"));
    if (ferr.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream( &f );
        stream << task->error << endl;
    }
    task->data->insert("tasks_unfinished",QString("%1").arg(task->data->value("tasks_unfinished","1").toInt()-1));
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
         connect(wrapper,SIGNAL(finished(TaskPtr,bool)),this,SLOT(onTaskFinished(TaskPtr,bool)));
         cpu_processes_.append(wrapper);

     }
     for(int i=0;i<num_gpu;++i) {
         int gpu_id=gpu_ids.at(i%gpu_ids.size()).toInt();
         ProcessWrapper* wrapper=new ProcessWrapper(this,timeout,gpu_id);
         emit processCreated(wrapper,gpu_id);
         connect(wrapper,SIGNAL(finished(TaskPtr,bool)),this,SLOT(onTaskFinished(TaskPtr,bool)));
         gpu_processes_.append(wrapper);
     }

    root_task_->children.clear();
    settings->beginGroup("Tasks");
    loadTask_(settings,root_task_);
    settings->endGroup();
    delete settings;

}

void ImageProcessor::exportImages(const QUrl &export_path, const QUrl &raw_export_path, const QStringList &image_list, const QStringList &output_keys, const QStringList &raw_keys, const QStringList &shared_keys, bool duplicate_raw)
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
        foreach(QString image,image_list){
            QStringList raw_list;
            foreach(QString key, raw_files_[image].keys()){
                if(raw_keys.contains(key)){
                    raw_list << raw_files_[image][key];
                }
            }
            process_->write(QString("raw_%1=%2\n").arg(image).arg(raw_list.join(",")).toLatin1());
            QStringList output_list;
            foreach(QString key, output_files_[image].keys()){
                if(output_keys.contains(key)){
                    output_list << output_files_[image][key];
                }
            }
            process_->write(QString("%1=%2\n").arg(image).arg(output_list.join(",")).toLatin1());
        }
        QStringList shared_list;
        foreach(QString key, shared_output_files_.keys()){
            if(shared_keys.contains(key)){
                shared_list << shared_output_files_[key];
            }
        }
        process_->write(QString("%1=%2\n").arg("shared").arg(shared_list.join(",")).toLatin1());
        process_->closeWriteChannel();
    }else{
        int num_processes=settings.value("export_num_processes",1).toInt();

        QStringList files;
        QStringList raw_files;
        QStringList files_to_filter;
        QString current_dir=QDir::current().dirName();
        foreach(QString key,shared_output_files_.keys()){
            if(shared_output_files_[key].endsWith(".star")){
                files_to_filter.append(current_dir+"/"+shared_output_files_[key]);
            }else{
                files.append(current_dir+"/"+shared_output_files_[key]);
            }
        }
        foreach(QString image,image_list){
            foreach(QString key, output_files_[image].keys()){
                if(output_keys.contains(key)){
                    files << current_dir+"/"+output_files_[image][key];
                }
            }
            foreach(QString key, raw_files_[image].keys()){
                if(raw_keys.contains(key)){
                    raw_files << current_dir+"/"+raw_files_[image][key];
                }
            }
        }
        bool separate_raw_export=export_path!=raw_export_path;
        if( (!separate_raw_export) || duplicate_raw){
            files.append(raw_files);
        }
        QDir parent_dir=QDir::current();
        parent_dir.cdUp();
        if(!files.empty()){
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
       if(! proc->running() &&  ! cpu_task_stack_.empty()){
           proc->start(cpu_task_stack_.pop());
           count_changed=true;
       }
    }
    foreach (ProcessWrapper* proc, gpu_processes_) {
       if(! proc->running() &&  ! gpu_task_stack_.empty()){
           proc->start(gpu_task_stack_.pop());
           count_changed=true;
       }
    }
    if(count_changed){
        emit queueCountChanged(cpu_task_stack_.size(),gpu_task_stack_.size());
    }
}

QSet<QString> ImageProcessor::getOutputFilesKeys() const
{
    QSet<QString> result;
    foreach(QString hash_key, output_files_.keys()){
        result.unite(QSet<QString>::fromList(output_files_[hash_key].keys()));
    }
    return result;
}

QSet<QString> ImageProcessor::getRawFilesKeys() const
{
    QSet<QString> result;
    foreach(QString hash_key, raw_files_.keys()){
        result.unite(QSet<QString>::fromList(raw_files_[hash_key].keys()));
    }
    return result;
}

QSet<QString> ImageProcessor::getSharedFilesKeys() const
{
    return QSet<QString>::fromList(shared_output_files_.keys());
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


void ImageProcessor::createTaskTree( DataPtr data)
{
    TaskPtr root_task=root_task_->clone();
    root_task->setData(data);
    for(int i=0;i<root_task->children.size();++i){
        QStack<TaskPtr>& stack=root_task->children.at(i)->gpu?gpu_task_stack_:cpu_task_stack_;
        stack.append(root_task->children.at(i));
    }
    emit queueCountChanged(cpu_task_stack_.size(),gpu_task_stack_.size());
    startTasks();
}


void ImageProcessor::loadTask_(Settings *settings, const TaskPtr &task)
{
    foreach(QString child_name,settings->childGroups()){
        settings->beginGroup(child_name);
        TaskPtr child(new Task(child_name,settings->value("script").toString(), DataPtr(new Data()),settings->value("is_gpu").toBool()));
        task->children.append(child);
        loadTask_(settings,child);
        settings->endGroup();
    }

}

