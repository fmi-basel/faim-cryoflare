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

#ifndef PROCESSWRAPPER_H
#define PROCESSWRAPPER_H

#include <QObject>
#include <QProcess>
#include <QHash>
#include "task.h"

//fw decl
class QTimer;

class ProcessWrapper : public QObject
{
    Q_OBJECT
public:
    explicit ProcessWrapper(QObject *parent, int timeout, int gpu_id);
    bool running() const;
    TaskPtr task() const;

signals:
    void finished(const TaskPtr &task, bool gpu=false);
    void started(const QString &image, const QString &task,int process_id);
    void stopped();
    void error(const TaskPtr &task);

public slots:
    void start(const TaskPtr &task);
    void onFinished(int exitcode);
    void onStarted();
    void onError(QProcess::ProcessError e);
    void kill();
    void terminate();
    void timeout();
private:
    QProcess *process_;
    TaskPtr task_;
    int timeout_;
    bool terminated_;
    int gpu_id_;
    QTimer* timeout_timer_;
  
};

#endif // PROCESSWRAPPER_H
