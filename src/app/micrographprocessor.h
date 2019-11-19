//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2017-2018 by the CryoFLARE Authors
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

#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QObject>
#include <QStringList>
#include <task.h>
#include <dataptr.h>
#include "processqueue.h"

//fw decl
class Settings;
class QProcess;
class MetaDataStore;
class DataSourceBase    ;

class MicrographProcessor: public QObject
{
    Q_OBJECT
public:
    MicrographProcessor(MetaDataStore* meta_data_store,DataSourceBase* data_source, TaskConfiguration* task_configuration);
    ~MicrographProcessor();
public slots:
    void startStop(bool start=true);
    void loadSettings();
    void processMicrograph(const QString &id);
    void reprocessMicrograph(const QString &id);
signals:
    void newImage(Data data);
    void imageUpdated(const QString& image);
    void queueCountChanged(int,int);
    void processesCreated(QList<ProcessWrapper*> processes);
    void processesDeleted();

protected slots:
    void enqueueChildren_(const QString &id, const TaskDefinitionPtr &taskdef);
    void handleTaskFailed(const QString &id, const TaskDefinitionPtr &taskdef,int exitcode);
private:
    QString epu_project_dir_;
    QString movie_dir_;
    ProcessQueue* cpu_queue_;
    ProcessQueue* gpu_queue_;
    MetaDataStore* meta_data_store_;
    DataSourceBase* data_source_;
    TaskConfiguration* task_configuration_;

};


#endif // IMAGEPROCESSOR_H
