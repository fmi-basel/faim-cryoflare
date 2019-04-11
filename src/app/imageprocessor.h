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

#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QStack>
#include <QObject>
#include <QStringList>
#include <task.h>
#include <parallelexporter.h>
#include <dataptr.h>
#include "sftpurl.h"

//fw decl
class ProcessWrapper;
class Settings;
class QProcess;
class MetaDataStore;

class ImageProcessor: public QObject
{
    Q_OBJECT
public:
    ImageProcessor(MetaDataStore &meta_data_store);
    ~ImageProcessor();
public slots:
    void startStop(bool start=true);
    void onTaskFinished(const TaskPtr& task, bool gpu);
    void onTaskError(const TaskPtr& task);
    void loadSettings();
    void exportImages(const SftpUrl& export_path,const SftpUrl& raw_export_path,const QStringList& image_list, const QStringList& output_keys,const QStringList& raw_keys,const QStringList& shared_keys,bool duplicate_raw );
    void cancelExport();
    void startTasks();
    void createTaskTree(DataPtr data, bool force_reprocess=false);
    void reprocess(const QVector<DataPtr>& images);
signals:
    void newImage(DataPtr data);
    void dataChanged(DataPtr data);
    void imageUpdated(const QString& image);
    void queueCountChanged(int,int);
    void processCreated(ProcessWrapper *wrapper,int gpu_id);
    void processesDeleted();
    void exportStarted(const QString& title, int num);
    void exportMessage(int left, const QList<ExportMessage>& output);
    void exportFinished();

protected slots:
    void exportFinished_();
private:
    void startNextExport_();
    void loadTask_(Settings *setting,const TaskPtr& task);
    void enqueueChildren_(const TaskPtr& task);

    QString epu_project_dir_;
    QString movie_dir_;
    QStack<TaskPtr> cpu_task_stack_;
    QStack<TaskPtr> gpu_task_stack_;
    QList<ProcessWrapper*> cpu_processes_;
    QList<ProcessWrapper*> gpu_processes_;
    TaskPtr root_task_;
    QQueue<ParallelExporter*> exporters_;
    ParallelExporter* current_exporter_;
    QProcess* process_;
    bool running_state_;
    MetaDataStore& meta_data_store_;


};


#endif // IMAGEPROCESSOR_H
