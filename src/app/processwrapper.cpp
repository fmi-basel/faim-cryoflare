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
    connect(process_,&QProcess::errorOccurred,this,&ProcessWrapper::onError);
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
    QVariantHash raw_files;
    if(task_->data->contains("raw_files")){
        raw_files=task_->data->value("raw_files").toObject().toVariantHash();
    }
    QVariantHash files;
    if(task_->data->contains("files")){
        files=task_->data->value("files").toObject().toVariantHash();
    }
    QVariantHash shared_files;
    if(task_->data->contains("shared_files")){
        shared_files=task_->data->value("shared_files").toObject().toVariantHash();
    }
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
            raw_files.insert(splitted[0].trimmed(),splitted[1].trimmed());
            //task_->raw_files.insert(splitted[0].trimmed(),splitted[1].trimmed());
        }else if(line.startsWith(result_file_token)){
            line.remove(0,result_file_token.size());
            QStringList splitted=line.split("=");
            if(splitted.size()<2){
                qDebug() << "invalid result line: " << line;
                continue;
            }
            files.insert(splitted[0].trimmed(),splitted[1].trimmed());
            //task_->output_files.insert(splitted[0].trimmed(),splitted[1].trimmed());
        }else if(line.startsWith(shared_result_file_token)){
            line.remove(0,shared_result_file_token.size());
            QStringList splitted=line.split("=");
            if(splitted.size()<2){
                qDebug() << "invalid result line: " << line;
                continue;
            }
            shared_files.insert(splitted[0].trimmed(),splitted[1].trimmed());
            //task_->shared_output_files.insert(splitted[0].trimmed(),splitted[1].trimmed());
        }
    } while (!line.isNull());
    task_->data->insert("raw_files",QJsonObject::fromVariantHash(raw_files));
    task_->data->insert("files",QJsonObject::fromVariantHash(files));
    task_->data->insert("shared_files",QJsonObject::fromVariantHash(shared_files));
    task_->output=output;
    task_->error=process_->readAllStandardError();
    task_->state=exitcode;
    TaskPtr task=task_;
    task_.clear();
    emit finished(task,-1!=gpu_id_);
}

void ProcessWrapper::onStarted()
{
    QByteArray script_input;
    foreach(QString key,task_->data->keys()){
        QString val=task_->data->value(key).toString();
        script_input.append(QString("%1=%2\n").arg(key,val).toLatin1());
    }
    if(-1!=gpu_id_){
        script_input.append(QString("gpu_id=%1\n").arg(gpu_id_).toLatin1());
    }
    Settings settings;
    settings.beginGroup("ScriptInput");
    settings.beginGroup(task_->name);
    foreach(QString name,settings.allKeys()){
        QString value=settings.value(name).toString();
        script_input.append(QString("%1=%2\n").arg(name,value).toLatin1());
    }
    settings.endGroup();
    settings.endGroup();
    process_->write(script_input);
    process_->closeWriteChannel();
    emit started(task_->name,task_->data->value("short_name").toString(),process_->processId());
}

void ProcessWrapper::onError(QProcess::ProcessError e)
{
    QString error_string;
    switch (e) {
        case QProcess::FailedToStart:
            error_string=QString("Task %1 failed to start").arg(task_->script);
            break;
        case QProcess::Crashed:
            error_string=QString("Task %1 crashed.").arg(task_->script);
            break;
        case QProcess::Timedout:
            error_string=QString("Task %1  waitFor...() function timed out.").arg(task_->script);
            break;
        case QProcess::WriteError:
            error_string=QString("An error occurred when attempting to write to task %1.").arg(task_->script);
            break;
        case QProcess::ReadError:
            error_string=QString("An error occurred when attempting to read from task %1.").arg(task_->script);
            break;
        case QProcess::UnknownError:
        default:
            error_string=QString("Unknown error in task %1.").arg(task_->script);
            break;
    }
    task_->output=process_->readAllStandardOutput();
    task_->error=process_->readAllStandardError()+"\n"+error_string;
    task_->state=e;
    TaskPtr task=task_;
    task_.clear();
    emit error(task);
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

TaskPtr ProcessWrapper::task() const
{
    return task_;
}
