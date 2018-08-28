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

#include <QtDebug>
#include <QProcessEnvironment>
#include <QCoreApplication>
#include <QTextStream>
#include <QTimer>
#include "settings.h"
#include "processwrapper.h"

ProcessWrapper::ProcessWrapper(QObject *parent, int timeout, int gpu_id) :
    QObject(parent),
    process_(new QProcess(this)),
    task_(),
    timeout_(timeout),
    terminated_(false),
    gpu_id_(gpu_id),
    timeout_timer_(new QTimer(this))
{
    connect(process_,SIGNAL(finished(int)),this,SLOT(onFinished(int)));
    connect(process_,SIGNAL(started()),this,SLOT(onStarted()));
    QProcessEnvironment env=QProcessEnvironment::systemEnvironment();
    QString path=env.value("PATH");
    env.insert("PATH",QCoreApplication::applicationDirPath()+"/scripts:"+path);
    process_->setProcessEnvironment(env);
    connect(timeout_timer_, SIGNAL(timeout()), this, SLOT(timeout()));
    timeout_timer_->setSingleShot(true);
}

bool ProcessWrapper::running() const
{
    return process_->state()!=QProcess::NotRunning;
}

void ProcessWrapper::start(const TaskPtr &task)
{
    task_=task;
    process_->start(task_->script);
    foreach(QString key,task_->data->keys()){
        QString val=task_->data->value(key);
        process_->write(QString("%1=%2\n").arg(key,val).toLatin1());
    }
    if(-1!=gpu_id_){
        process_->write(QString("gpu_id=%1\n").arg(gpu_id_).toLatin1());
    }
    Settings settings;
    settings.beginGroup("ScriptInput");
    settings.beginGroup(task->name);
    foreach(QString name,settings.allKeys()){
        QString value=settings.value(name).toString();
        process_->write(QString("%1=%2\n").arg(name,value).toLatin1());
    }
    settings.endGroup();
    settings.endGroup();
    process_->closeWriteChannel();
    if(timeout_>0){
        timeout_timer_->start(timeout_*1000);
    }
}

void ProcessWrapper::onFinished(int exitcode)
{
    emit stopped();
    timeout_timer_->stop();
    if(terminated_){
        process_->readAllStandardError();
        process_->readAllStandardOutput();
        terminated_=false;
        return;
    }
    QTextStream output_stream(process_->readAllStandardOutput(),QIODevice::ReadOnly);
    QString result_token("RESULT_EXPORT:");
    QString raw_file_token("RAW_FILE_EXPORT:");
    QString result_file_token("FILE_EXPORT:");
    QString shared_result_file_token("SHARED_FILE_EXPORT:");
    QString output;
    QString line;
    do {
        line = output_stream.readLine();
        output.append(line+"\n");
        if(line.startsWith(result_token)){
            line.remove(0,result_token.size());
            QStringList splitted=line.split("=");
            if(splitted.size()<2){
                qDebug() << "invalid result line: " << line;
                continue;
            }
            task_->data->insert(splitted[0].trimmed(),splitted[1].trimmed());
        }else if(line.startsWith(raw_file_token)){
            line.remove(0,raw_file_token.size());
            QStringList splitted=line.split("=");
            if(splitted.size()<2){
                qDebug() << "invalid result line: " << line;
                continue;
            }
            task_->raw_files.insert(splitted[0].trimmed(),splitted[1].trimmed());
        }else if(line.startsWith(result_file_token)){
            line.remove(0,result_file_token.size());
            QStringList splitted=line.split("=");
            if(splitted.size()<2){
                qDebug() << "invalid result line: " << line;
                continue;
            }
            task_->output_files.insert(splitted[0].trimmed(),splitted[1].trimmed());
        }else if(line.startsWith(shared_result_file_token)){
            line.remove(0,shared_result_file_token.size());
            QStringList splitted=line.split("=");
            if(splitted.size()<2){
                qDebug() << "invalid result line: " << line;
                continue;
            }
            task_->shared_output_files.insert(splitted[0].trimmed(),splitted[1].trimmed());
        }
    } while (!line.isNull());
    task_->output=output;
    task_->error=process_->readAllStandardError();
    task_->state=exitcode;
    TaskPtr task=task_;
    task_.clear();
    emit finished(task,-1!=gpu_id_);
}

void ProcessWrapper::onStarted()
{
    emit started(task_->name,task_->data->value("short_name"),process_->processId());
}

void ProcessWrapper::kill()
{
    process_->kill();
    process_->waitForFinished();
}

void ProcessWrapper::terminate()
{
    if(!running()){
        return;
    }
    terminated_=true;
    process_->terminate();
    process_->waitForFinished();

}

void ProcessWrapper::timeout()
{
    if(!running()){
        return;
    }
    terminate();
    process_->waitForFinished();
    start(task_);
}
