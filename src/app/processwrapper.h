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

#ifndef PROCESSWRAPPER_H
#define PROCESSWRAPPER_H

#include <QObject>
#include <QProcess>
#include <QHash>
#include "task.h"
#include "metadatastore.h"

//fw decl
class QTimer;

class ProcessWrapper : public QObject
{
    Q_OBJECT
public:
    explicit ProcessWrapper(QObject *parent, MetaDataStore* meta_data_store,int timeout, int gpu_id);
    bool running() const;
    TaskPtr task() const;
    int gpuID() const;

signals:
    void finished(ProcessWrapper* process, const TaskPtr &task,int exitcode);
    void started(const QString &image, const QString &task,int process_id);
    void error(const TaskPtr &task);

public slots:
    void start(const TaskPtr &task);
    void kill();
    void terminate();

private slots:
    void onStarted_();
    void onFinished_(int exitcode, QProcess::ExitStatus state);
    void onError_(QProcess::ProcessError e);
private:
    void handleSuccess_();
    void handleFailure_();
    void writeLog_(const QString& text);
    void writeErrorLog_(const QString& text);
    QProcess *process_;
    TaskPtr task_;
    int timeout_;
    int gpu_id_;
    QTimer* timeout_timer_;
    MetaDataStore* meta_data_store_;

};

#endif // PROCESSWRAPPER_H
