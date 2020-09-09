//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2017-2019 by the CryoFLARE Authors
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

#include <QtDebug>
#include <QProcessEnvironment>
#include <QCoreApplication>
#include <QTextStream>
#include <QTimer>
#include <QDir>
#include "settings.h"
#include "processwrapper.h"

ProcessWrapper::ProcessWrapper(QObject *parent, MetaDataStore* meta_data_store, int timeout, int gpu_id) :
    QObject(parent),
    process_(new QProcess(this)),
    task_(),
    timeout_(timeout),
    gpu_id_(gpu_id),
    timeout_timer_(new QTimer(this)),
    meta_data_store_(meta_data_store)
{
    connect(process_,&QProcess::started,this,&ProcessWrapper::onStarted_);
    connect(process_,QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished) ,this,&ProcessWrapper::onFinished_);
    connect(timeout_timer_, &QTimer::timeout, this, &ProcessWrapper::terminate);
    connect(process_,&QProcess::errorOccurred,this,&ProcessWrapper::onError_);
    QProcessEnvironment env=QProcessEnvironment::systemEnvironment();
    QString path=env.value("PATH");
    env.insert("PATH",QCoreApplication::applicationDirPath()+"/scripts:"+path);
    QString pythonpath=env.value("PYTHONPATH");
    env.insert("PYTHONPATH",QCoreApplication::applicationDirPath()+"/scripts:"+pythonpath);
    process_->setProcessEnvironment(env);
    timeout_timer_->setSingleShot(true);
}

bool ProcessWrapper::running() const
{
    return process_->state()!=QProcess::NotRunning;
}

void ProcessWrapper::start(const TaskPtr &task)
{
    task_=task;
    process_->start(task_->definition->script);
    if(timeout_>0){
        timeout_timer_->start(timeout_*1000);
    }
}

void ProcessWrapper::onStarted_()
{
    QByteArray script_input;
    Data data=meta_data_store_->micrograph(task_->id);
    foreach(QString key,data.keys()){
        QString val=data.value(key).toString();
        script_input.append(QString("%1=%2\n").arg(key,val).toLatin1());
    }
    if(-1!=gpu_id_){
        script_input.append(QString("gpu_id=%1\n").arg(gpu_id_).toLatin1());
    }
    QString hole_id=data.parent();
    if(meta_data_store_->hasFoilhole(hole_id)){
        Data hole_data=meta_data_store_->foilhole(hole_id);
        foreach(QString key,hole_data.keys()){
            QString val=hole_data.value(key).toString();
            script_input.append(QString("hole_%1=%2\n").arg(key,val).toLatin1());
        }
        QString square_id=hole_data.parent();
        if(meta_data_store_->hasGridsquare(square_id)){
            Data grid_data=meta_data_store_->gridsquare(square_id);
            foreach(QString key,grid_data.keys()){
                QString val=grid_data.value(key).toString();
                script_input.append(QString("grid_%1=%2\n").arg(key,val).toLatin1());
            }
        }
    }
    Settings settings;
    settings.beginGroup("ScriptInput");
    settings.beginGroup(task_->definition->name);
    foreach(QString name,settings.allKeys()){
        QString value=settings.value(name).toString();
        script_input.append(QString("%1=%2\n").arg(name,value).toLatin1());
    }
    settings.endGroup();
    settings.endGroup();
    process_->write(script_input);
    process_->closeWriteChannel();
    emit started(task_->definition->name,data.value("short_name").toString(),process_->processId());
}


void ProcessWrapper::onFinished_(int exitcode, QProcess::ExitStatus state)
{
    timeout_timer_->stop();
    if(state==QProcess::NormalExit && exitcode==0){
        handleSuccess_();
    }else{
        handleFailure_();
    }
}


void ProcessWrapper::onError_(QProcess::ProcessError e)
{
    QString error_string;
    switch (e) {
        case QProcess::FailedToStart:
            error_string=QString("Task %1 failed to start\n").arg(task_->definition->script);
            break;
        case QProcess::Crashed:
            error_string=QString("Task %1 crashed.\n").arg(task_->definition->script);
            break;
        case QProcess::Timedout:
            error_string=QString("Task %1  waitFor...() function timed out.\n").arg(task_->definition->script);
            break;
        case QProcess::WriteError:
            error_string=QString("An error occurred when attempting to write to task %1.\n").arg(task_->definition->script);
            break;
        case QProcess::ReadError:
            error_string=QString("An error occurred when attempting to read from task %1.\n").arg(task_->definition->script);
            break;
        case QProcess::UnknownError:
        default:
            error_string=QString("Unknown error in task %1.\n").arg(task_->definition->script);
            break;
    }
    writeErrorLog_(error_string);
    if(running()){
        terminate();
    }else{
        handleFailure_();
    }
}

void ProcessWrapper::handleSuccess_()
{
    QTextStream output_stream(process_->readAllStandardOutput(),QIODevice::ReadOnly);
    QString result_token("RESULT_EXPORT:");
    QString raw_file_token("RAW_FILE_EXPORT:");
    QString result_file_token("FILE_EXPORT:");
    QString shared_result_file_token("SHARED_FILE_EXPORT:");
    QString shared_raw_result_file_token("SHARED_RAW_FILE_EXPORT:");
    QString output;
    QString line;
    QMap<QString,QString> data,raw_files,files,shared_files,shared_raw_files;
    int linecount=0;
    do {
        line = output_stream.readLine();
        output.append(QString("%1\n").arg(line));
        ++linecount;
        if(line.startsWith(result_token)){
            line.remove(0,result_token.size());
            QStringList splitted=line.split("=");
            if(splitted.size()<2){
                qDebug() << "invalid result line: " << line;
                continue;
            }
            data.insert(splitted[0].trimmed(),splitted[1].trimmed());
        }else if(line.startsWith(raw_file_token)){
            line.remove(0,raw_file_token.size());
            QStringList splitted=line.split("=");
            if(splitted.size()<2){
                qDebug() << "invalid result line: " << line;
                continue;
            }
            raw_files.insert(splitted[0].trimmed(),splitted[1].trimmed());
        }else if(line.startsWith(result_file_token)){
            line.remove(0,result_file_token.size());
            QStringList splitted=line.split("=");
            if(splitted.size()<2){
                qDebug() << "invalid result line: " << line;
                continue;
            }
            files.insert(splitted[0].trimmed(),splitted[1].trimmed());
        }else if(line.startsWith(shared_result_file_token)){
            line.remove(0,shared_result_file_token.size());
            QStringList splitted=line.split("=");
            if(splitted.size()<2){
                qDebug() << "invalid result line: " << line;
                continue;
            }
            shared_files.insert(splitted[0].trimmed(),splitted[1].trimmed());
        }else if(line.startsWith(shared_raw_result_file_token)){
            line.remove(0,shared_raw_result_file_token.size());
            QStringList splitted=line.split("=");
            if(splitted.size()<2){
                qDebug() << "invalid result line: " << line;
                continue;
            }
            shared_raw_files.insert(splitted[0].trimmed(),splitted[1].trimmed());
        }
    } while (!line.isNull());
    data.insert(task_->definition->taskString(),"FINISHED");
    meta_data_store_->updateMicrograph(task_->id,data,raw_files,files,shared_files,shared_raw_files);
    writeLog_(output);
    writeErrorLog_(process_->readAllStandardError());
    TaskPtr task=task_;
    task_.clear();
    emit finished(this,task,0);
}

void ProcessWrapper::handleFailure_()
{
    process_->setReadChannel(QProcess::StandardError);
    if(process_->bytesAvailable()>0){
        writeErrorLog_(process_->readAllStandardError());
    }
    process_->setReadChannel(QProcess::StandardOutput);
    if(process_->bytesAvailable()>0){
        writeLog_(process_->readAllStandardOutput());
    }
    meta_data_store_->updateMicrograph(task_->id, {{ task_->definition->taskString(),"ERROR"}});
    TaskPtr task=task_;
    task_.clear();
    if(process_->exitStatus()==QProcess::NormalExit){
        emit finished(this,task,process_->exitCode());
    }else{
        emit finished(this,task,-1);
    }

}

void ProcessWrapper::writeLog_(const QString &text)
{
    QFile f(QDir::current().relativeFilePath(task_->definition->name+"_out.log"));
    if (f.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream( &f );
        foreach(QString line, text.split("\n", QString::SkipEmptyParts)){
            stream << task_->id << ": " << line << "\n";
        }
    }
}

void ProcessWrapper::writeErrorLog_(const QString &text)
{
    QFile ferr(QDir::current().relativeFilePath(task_->definition->name+"_error.log"));
    if (ferr.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream( &ferr );
        foreach(QString line, text.split("\n", QString::SkipEmptyParts)){
            stream << task_->id << ": " << line << "\n";
        }
    }
}

void ProcessWrapper::kill()
{
    process_->kill();
}

void ProcessWrapper::terminate()
{
    if(running()){
        process_->terminate();
    }
}


TaskPtr ProcessWrapper::task() const
{
    return task_;
}

int ProcessWrapper::gpuID() const
{
    return gpu_id_;
}
