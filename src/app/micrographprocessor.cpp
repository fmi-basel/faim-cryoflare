//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2017-2020 by the CryoFLARE Authors
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

#include <iostream>
#include <QApplication>
#include <QDir>
#include <QDateTime>
#include "settings.h"
#include <QtDebug>
#include <QProcess>
#include <processwrapper.h>
#include "micrographprocessor.h"
#include "metadatastore.h"
#include "datasourcebase.h"

MicrographProcessor::MicrographProcessor(MetaDataStore *meta_data_store, TaskConfiguration *task_configuration):
    QObject(),
    epu_project_dir_(),
    movie_dir_(),
    cpu_queue_(new ProcessQueue(meta_data_store,this)),
    gpu_queue_(new ProcessQueue(meta_data_store,this)),
    meta_data_store_(meta_data_store),
    task_configuration_(task_configuration)

{
    connect(meta_data_store_,&MetaDataStore::newMicrograph,this,&MicrographProcessor::processMicrograph);
    QTimer::singleShot(0, this, &MicrographProcessor::loadSettings);
    connect(cpu_queue_,&ProcessQueue::taskSucceded,this,&MicrographProcessor::enqueueChildren_);
    connect(cpu_queue_,&ProcessQueue::taskFailed,this,&MicrographProcessor::handleTaskFailed);
    connect(gpu_queue_,&ProcessQueue::taskSucceded,this,&MicrographProcessor::enqueueChildren_);
    connect(gpu_queue_,&ProcessQueue::taskFailed,this,&MicrographProcessor::handleTaskFailed);
}

MicrographProcessor::~MicrographProcessor()
{
}

void MicrographProcessor::startStop(bool start)
{
    if(start){
        Settings settings;
        epu_project_dir_=settings.value("avg_source_dir").toString();
        movie_dir_=settings.value("stack_source_dir").toString();
        meta_data_store_->start(epu_project_dir_,movie_dir_);
        cpu_queue_->start();
        gpu_queue_->start();
    }else{
        meta_data_store_->stop();
        cpu_queue_->stop();
        gpu_queue_->stop();
    }
}


void MicrographProcessor::loadSettings()
{
     emit processesDeleted();
     QScopedPointer<Settings> settings(new Settings);
     int num_cpu=settings->value("num_cpu",10).toInt();
     int num_gpu=settings->value("num_gpu",2).toInt();
     int timeout=settings->value("timeout",300).toInt();
     QStringList gpu_ids=settings->value("gpu_ids","0").toString().split(",", Qt::SkipEmptyParts);
     if(gpu_ids.empty()){
         for(int i=0;i<num_gpu;++i) {
             gpu_ids << QString("%1").arg(i);
         }
     }
     cpu_queue_->createProcesses(num_cpu,timeout,QStringList());
     emit processesCreated(cpu_queue_->processes_);
     gpu_queue_->createProcesses(num_gpu,timeout,gpu_ids);
     emit processesCreated(gpu_queue_->processes_);

}




void MicrographProcessor::processMicrograph(const QString &id)
{
    enqueueChildren_(id,task_configuration_->rootDefinition());
}

void MicrographProcessor::reprocessMicrograph(const QString &id)
{
    cpu_queue_->stopProcess(id);
    gpu_queue_->stopProcess(id);
    meta_data_store_->removeMicrographResults(id, task_configuration_->rootDefinition());
    processMicrograph(id);
}



void MicrographProcessor::enqueueChildren_(const QString& id, const TaskDefinitionPtr &taskdef)
{
    for(int i=0;i<taskdef->children.size();++i){
        TaskDefinitionPtr child=taskdef->children.at(i);
        Data data=meta_data_store_->micrograph(id);
        if(data.value(child->taskString()).toString()=="FINISHED"){
            enqueueChildren_(id,child);
        }else{
            ProcessQueue* queue=child->gpu?gpu_queue_:cpu_queue_;
            queue->enqueue(TaskPtr(new Task(child,id)));
        }
    }
    emit queueCountChanged(cpu_queue_->size(),gpu_queue_->size());
}

void MicrographProcessor::handleTaskFailed(const QString &id, const TaskDefinitionPtr &taskdef, int exitcode)
{
    Q_UNUSED(id)
    Q_UNUSED(taskdef)
    Q_UNUSED(exitcode)
    emit queueCountChanged(cpu_queue_->size(),gpu_queue_->size());
}

