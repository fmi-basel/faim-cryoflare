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

#include <unistd.h>
#include <QDir>
#include <QDebug>
#include <QApplication>
#include <QProgressDialog>
#include <QProcess>
#include <QThread>
#include <QSet>
#include <QtAlgorithms>
#include "parallelexporter.h"
#include "exportprogressdialog.h"

ExportWorkerBase::ExportWorkerBase(int id, QSharedPointer<ThreadSafeQueue<WorkItem> > queue, const QString &source, const QUrl &destination, const QStringList &images, Barrier<ThreadSafeQueue<WorkItem> > &b):
    QObject(),
    queue_(queue),
    source_(source),
    destination_(destination),
    images_(images),
    image_name_("\\d{8}_\\d{6}"),
    message_buffer_(),
    barrier_(b),
    id_(id),
    busy_(false)
{
    // queued connection here to allow signal to reach main event loop as well
    connect(this,&ExportWorkerBase::next,this,&ExportWorkerBase::processNext_,Qt::QueuedConnection);
}

ExportWorkerBase::~ExportWorkerBase()
{

}


bool ExportWorkerBase::busy() const
{
    return busy_;
}


QList<ExportMessage> ExportWorkerBase::messages()
{
    QList<ExportMessage> result=message_buffer_.list();
    message_buffer_.clear();
    return result;
}

void ExportWorkerBase::startExport()
{
    startImpl_();
}

void ExportWorkerBase::error_(const QString &error)
{
    message_buffer_.append(ExportMessage(id_,ExportMessage::ERROR,error));
}

void ExportWorkerBase::message_(const QString &message)
{
    message_buffer_.append(ExportMessage(id_,ExportMessage::MESSAGE,message));
}

QByteArray ExportWorkerBase::filter_(const QString &source_path) const
{
    QByteArray filtered_data;
    QTextStream out_stream(filtered_data);
    QFile file(source_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return filtered_data;
    }
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        QRegularExpressionMatch match = image_name_.match(line);
        if(match.hasMatch()){
            if(images_.contains(match.captured(0))){
                out_stream<<line;
            }
        }else{
            out_stream<<line;
        }
    }
    return filtered_data;
}

int ExportWorkerBase::id() const
{
    return id_;
}

void ExportWorkerBase::processNext_()
{
    processNextImpl_();
}


LocalExportWorker::LocalExportWorker(int id, QSharedPointer<ThreadSafeQueue<WorkItem> > queue, const QString &source, const QUrl &destination, const QStringList &images, Barrier<ThreadSafeQueue<WorkItem> > &b):
    ExportWorkerBase(id,queue,source,destination,images,b)
{

}

void LocalExportWorker::startImpl_()
{
    if(!busy()){
        processNext_();
    }
}

void LocalExportWorker::processNextImpl_()
{
    busy_=true;
    ThreadSafeQueue<WorkItem>::result_pair result=queue_->checkEmptyAndFirst();
    if(result.first){
        busy_=false;
        return;
    }
    if(result.second.type==WorkItem::BARRIER){
        barrier_.wait();
    }else{
        result=queue_->checkEmptyAndDequeue();
        if(result.first){
            busy_=false;
            return;
        }
        WorkItem item=result.second;
        switch(item.type){
        case WorkItem::FILE:
            copyFile_(item);
            break;
        case WorkItem::DIRECTORY:
            createDirectory_(item);
            break;
        default:
            break;
        }
    }
    emit next();
}
void LocalExportWorker::copyFile_(const WorkItem &item)
{
    QString path=source_.absoluteFilePath(item.filename);
    if(!QFileInfo(path).exists()){
        error_(QString("Skipping missing file: %1").arg(item.filename));
    }else{
        QString destination_file(QDir(destination_.path()).absoluteFilePath(item.filename));
        if(QFile::exists(destination_file)){
            QFile::remove(destination_file);
        }
        // filter file
        QSharedPointer<QFile> source_file;
        if(item.filter){
            QByteArray data=filter_(path);
            QFile dest(destination_file);
            if (!dest.open(QIODevice::WriteOnly | QIODevice::Text)){
                error_("Error filering and copying file: "+item.filename);
            }else{
                dest.write(data);
                message_("Copied and filtered: "+item.filename);
            }
        }else{
            QFile source_file(path);
            if(!source_file.copy(destination_file)){
                error_("Error copying file: "+item.filename);
            }else{
                message_("Copied: "+item.filename);
            }
        }
    }
}

void LocalExportWorker::createDirectory_(const WorkItem& item)
{
    foreach(QString dirname, item.directories){
        QString abs_dir_name=QDir(destination_.path()).absoluteFilePath(dirname);
        QDir dir(abs_dir_name);
        if(!dir.exists()){
            QString dirname=dir.dirName();
            dir.cdUp();
            if(!dir.mkpath(dirname)){
                error_("Error creating directory: "+dirname);
            }
        }
    }
}


RemoteExportWorker::RemoteExportWorker(int id, QSharedPointer<ThreadSafeQueue<WorkItem> > queue, const QString &source, const QUrl &destination, const QStringList &images, Barrier<ThreadSafeQueue<WorkItem> > &b):
    ExportWorkerBase(id,queue,source,destination,images,b),
    sftp_session_()
{
}

void RemoteExportWorker::startImpl_()
{
    if(!busy()){
        if(!sftp_session_.isValid()){
            message_("Initializing connection");
            if(!sftp_session_.connect(destination_)){
                error_("SFTP session inizialization failed");
                return;
            }
        }
        processNext_();
    }
}


void RemoteExportWorker::processNextImpl_()
{
    busy_=true;
    ThreadSafeQueue<WorkItem>::result_pair result=queue_->checkEmptyAndFirst();
    if(result.first){
        busy_=false;
        return;
    }
    if(result.second.type==WorkItem::BARRIER){
        barrier_.wait();
    }else{
        result=queue_->checkEmptyAndDequeue();
        if(result.first){
            busy_=false;
            return;
        }
        WorkItem current_item=result.second;
        switch(current_item.type){
        case WorkItem::FILE:
            copyFile_(current_item);
            break;
        case WorkItem::DIRECTORY:
            createDirectory_(current_item);
            break;
        default:
            break;
        }
    }
    emit next();
}

void RemoteExportWorker::copyFile_(const WorkItem &item)
{
    QString path=source_.absoluteFilePath(item.filename);
    if(!QFileInfo(path).exists()){
        error_(QString("Skipping missing file: %1").arg(item.filename));
        emit next();
        return;
    }
    if(QFileInfo(path).isSymLink()){
        QByteArray buffer(2048,0);
        ssize_t len=readlink(source_.absoluteFilePath(item.filename).toLatin1().data(), buffer.data(), static_cast<size_t>(buffer.size()));
        if(len>0){
            QString target=QString::fromLatin1(buffer.data(),len);
            sftp_session_.createLink(QString(buffer),QDir(destination_.path()).absoluteFilePath(item.filename));
        }else{
            error_(QString("Error reading local symlink: %1").arg(item.filename));
        }
    }else{
        if(item.filter){
            QByteArray data=filter_(path);
            QDataStream data_stream(data);
            if(sftp_session_.writeFile(data_stream, QDir(destination_.path()).absoluteFilePath(item.filename))){
                message_(QString("Copied and filtered file: %1").arg(QDir(destination_.path()).absoluteFilePath(item.filename)));
            }else{
                error_(QString("Couldn't copy and filter file: %1 (error %2)").arg(QDir(destination_.path()).absoluteFilePath(item.filename)).arg(sftp_session_.getError()));
            }
        }else{
            if(sftp_session_.writeFile(path, QDir(destination_.path()).absoluteFilePath(item.filename))){
                message_(QString("Copied file: %1").arg(QDir(destination_.path()).absoluteFilePath(item.filename)));
            }else{
                error_(QString("Couldn't copy file: %1 (error %2)").arg(QDir(destination_.path()).absoluteFilePath(item.filename)).arg(sftp_session_.getError()));
            }
        }
    }
}

void RemoteExportWorker::createDirectory_(const WorkItem &item)
{
    QString dir_path(QDir(destination_.path()).absoluteFilePath(item.directories.first()));
    if(!sftp_session_.isDir(dir_path)){
        if(!sftp_session_.mkDir(dir_path)){
            error_(QString("Couldn't create directory: %1 (error %2)").arg(dir_path).arg(sftp_session_.getError()));
        }

    }
}

ParallelExporter::ParallelExporter(const QString &source, const QUrl &destination, const QStringList& images, int num_threads, QObject *parent):
    QObject(parent),
    source_(source),
    destination_(destination),
    images_(images),
    num_threads_(num_threads),
    queue_(new ThreadSafeQueue<WorkItem>()),
    started_(false),
    workers_(),
    message_timer_(),
    barrier_(queue_.data(),num_threads),
    export_progress_dialog_(new ExportProgressDialog())
{
    for(int i=0;i<num_threads_ ;++i){
        QThread* thread = new QThread();
        ExportWorkerBase* w;
        if(destination.isLocalFile()){
            w=new LocalExportWorker(i+1, queue_,source, destination, images,barrier_);
        }else{
            w=new RemoteExportWorker(i+1, queue_,source, destination, images,barrier_);
        }
        workers_.append(w);
        w->moveToThread(thread);
        connect(this,&ParallelExporter::newFiles,w,&ExportWorkerBase::startExport);
        // Proper connect sequence crucial here
        // See https://mayaposch.wordpress.com/2011/11/01/how-to-really-truly-use-qthreads-the-full-explanation/ for details
        connect(this,&ParallelExporter::deleteThreads,thread,&QThread::quit);
        connect(this,&ParallelExporter::deleteThreads,w,&ExportWorkerBase::deleteLater);
        connect(thread,&QThread::finished,thread,&QThread::deleteLater);
        thread->start();
    }
    message_timer_.start(1000);
    connect(&message_timer_,&QTimer::timeout,this,&ParallelExporter::getMessages);
    connect(export_progress_dialog_,&ExportProgressDialog::rejected,this,&ParallelExporter::cancel);

}

ParallelExporter::~ParallelExporter()
{
    emit deleteThreads();
}

void ParallelExporter::addImages(const QStringList &files, bool filter)
{
    QList<WorkItem> work_items;
    QHash<QString,QSet<QString>> directories;
    foreach(QString f,files){
        QString path=QFileInfo(f).path();
        if(path=="."){
            continue;
        }
        QStringList splitted_path=path.split("/",Qt::SkipEmptyParts);
        if(!directories.contains(splitted_path[0])){
            directories.insert(splitted_path[0],QSet<QString>());
        }
        for(int i=1;i<=splitted_path.size();++i){
            directories[splitted_path[0]].insert(splitted_path.mid(0,i).join("/"));
        }
    }
    if(!directories.empty()){
        work_items.append(WorkItem::createBarrier());
        foreach(QString key,directories.keys()){
            QList<QString> dir_paths=directories[key].values();
            std::sort(dir_paths.begin(),dir_paths.end());
            work_items.append(WorkItem::createDirectory(dir_paths));
        }
        work_items.append(WorkItem::createBarrier());
    }
    foreach(QString f,files){
        if(filter){
            work_items.append(WorkItem::createFile(f,true));
        }else{
            work_items.append(WorkItem::createFile(f,false));
        }
    }
    queue_->append(work_items);
    if(started_){
        emit newFiles();
    }
}

QUrl ParallelExporter::destination() const
{
    return destination_;
}

int ParallelExporter::numFiles() const
{
    return queue_->size();
}

void ParallelExporter::cancel()
{
    queue_->clear();
    barrier_.release();
    emit finished();
}

void ParallelExporter::start()
{
    started_=true;
    export_progress_dialog_->start(destination().toString(QUrl::RemovePassword),numFiles());
    emit newFiles();
}

void ParallelExporter::getMessages()
{
    int left=numFiles();
    int n_busy=0;
    QList<ExportMessage> messages;
    foreach(ExportWorkerBase* w, workers_){
        if(w->busy()){
            ++n_busy;
        }
        messages.append(w->messages());
    }
    export_progress_dialog_->update(messages,left+n_busy);
    if(left==0 && n_busy==0){
        export_progress_dialog_->finish();
        emit finished();
    }
}


