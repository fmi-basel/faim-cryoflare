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
#ifndef PROCESSQUEUE_H
#define PROCESSQUEUE_H

#include <QObject>
#include "priorityqueue.h"
#include "metadatastore.h"

//fw decl
class ProcessWrapper;

class ProcessQueue : public QObject
{
    Q_OBJECT
public:
    explicit ProcessQueue(MetaDataStore* meta_data_store,QObject *parent = nullptr);
    virtual ~ProcessQueue();
    void enqueue(const TaskPtr& task);
    void start();
    void stop();
    void stopProcess(const QString& id);
    int size();
    void createProcesses(int num_processes=0, int timeout=300,QStringList gpus=QStringList());
    QList<ProcessWrapper*> processes_;
    QList<ProcessWrapper*> idle_processes_;
    PriorityQueue pqueue_;
    bool running_;
protected:
    MetaDataStore * meta_data_store_;
signals:
    void taskSucceded(const QString &id, const TaskDefinitionPtr &taskdef);
    void taskFailed(const QString &id, const TaskDefinitionPtr &taskdef, int exitcode);
public slots:
    void onProcessFinished(ProcessWrapper* process, const TaskPtr &task, int exitcode);
};

#endif // PROCESSQUEUE_H
