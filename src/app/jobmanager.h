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
#ifndef JOBMANAGER_H
#define JOBMANAGER_H

#include <QList>
#include <QStack>
#include <QObject>
#include "job.h"

struct Core{
    int pid;
    int start_time;
};


class JobManager : public QObject
{
    Q_OBJECT
public:
    explicit JobManager(QObject *parent = nullptr);
    void submit( Job& j);

signals:

public slots:
    void jobFinished(const Job& j);
protected:
    void start_(Job &j);
    int numCoresFree_(QList<Core>& core_list) const;
    QVector<int> acquireCores_(QList<Core>& core_list,int num);
    void releaseCores_(QList<Core>& core_list,const QVector<int>& ids);
protected:
    QList<Core> cpu_cores_;
    QList<Core> gpu_cores_;
    QList<Job>  running_jobs_;
    QStack<Job> gpu_queue_;
    QStack<Job> cpu_queue_;

};

#endif // JOBMANAGER_H
