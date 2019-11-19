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

#include <unistd.h>
#include <iostream>
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

ExportWorkerBase::ExportWorkerBase(int id, QSharedPointer<ThreadSafeQueue<WorkItem> > queue, const QString &source, const SftpUrl &destination, const QStringList &images, Barrier<ThreadSafeQueue<WorkItem> > &b):
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

QTemporaryFile* ExportWorkerBase::filter_(const QString &source_path) const
{
    QTemporaryFile* temp_file=new QTemporaryFile();
    temp_file->open();
    temp_file->setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ReadGroup);
    QFile file(source_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        return nullptr;
    }
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        QRegularExpressionMatch match = image_name_.match(line);
        if(match.hasMatch()){
            if(images_.contains(match.captured(0))){
                temp_file->write(line);
            }
        }else{
            temp_file->write(line);
        }
    }
    temp_file->flush();
    return temp_file;
}

int ExportWorkerBase::id() const
{
    return id_;
}

void ExportWorkerBase::processNext_()
{
    processNextImpl_();
}


LocalExportWorker::LocalExportWorker(int id, QSharedPointer<ThreadSafeQueue<WorkItem> > queue, const QString &source, const SftpUrl &destination, const QStringList &images, Barrier<ThreadSafeQueue<WorkItem> > &b):
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
        // filter file
        QSharedPointer<QFile> source_file;
        if(item.filter){
            QTemporaryFile* temp_file=filter_(path);
            if(!temp_file){
                return;
            }
            source_file=QSharedPointer<QFile>(temp_file);
        }else{
            source_file=QSharedPointer<QFile>(new QFile(path));
        }
        if(!source_file->copy(QDir(destination_.path()).absoluteFilePath(item.filename))){
            error_("Error copying file: "+item.filename);
        }else{
            message_("Copied: "+item.filename);
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


RemoteExportWorker::RemoteExportWorker(int id, QSharedPointer<ThreadSafeQueue<WorkItem> > queue, const QString &source, const SftpUrl &destination, const QStringList &images, Barrier<ThreadSafeQueue<WorkItem> > &b):
    ExportWorkerBase(id,queue,source,destination,images,b),
    connection_parameters_(destination.toConnectionParameters()),
    ssh_connection_(nullptr),
    channel_(),
    sftp_ops_(),
    current_item_(),
    temp_files_()
{
}

void RemoteExportWorker::startImpl_()
{
    if(!busy()){
        if(ssh_connection_){
            switch(ssh_connection_->state()){
            case QSsh::SshConnection::Unconnected:
                ssh_connection_->connectToHost();
                break;
            case QSsh::SshConnection::Connecting:
                break;
            case QSsh::SshConnection::Connected:
                processNext_();
                break;
            }
        }else{
            message_("Starting connection");
            ssh_connection_=new QSsh::SshConnection(connection_parameters_, this);
            connect(ssh_connection_,&QSsh::SshConnection::connected,this,&RemoteExportWorker::connected_);
            connect(ssh_connection_,&QSsh::SshConnection::error,this,&RemoteExportWorker::connectionFailed_);
            ssh_connection_->connectToHost();
        }
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
        emit next();
    }else{
        result=queue_->checkEmptyAndDequeue();
        if(result.first){
            busy_=false;
            return;
        }
        current_item_=result.second;
        switch(current_item_.type){
        case WorkItem::FILE:
            copyFile_();
            break;
        case WorkItem::DIRECTORY:
            checkDestinationDirectory_();
            break;
        default:
            break;
        }
    }
}

void RemoteExportWorker::copyFile_()
{
    QString path=source_.absoluteFilePath(current_item_.filename);
    if(!QFileInfo(path).exists()){
        error_(QString("Skipping missing file: %1").arg(current_item_.filename));
        emit next();
        return;
    }
    if(QFileInfo(path).isSymLink()){
        // check for existence of symlink at destination
        QSsh::SftpJobId id=channel_->statFile(QDir(destination_.path()).absoluteFilePath(current_item_.filename));
        sftp_ops_.insert(id,LinkStat);
    }else{
        if(current_item_.filter){
            QTemporaryFile* temp_file=filter_(path);
            if(!temp_file){
                error_(QString("Could not create temporary file for filtering: %1").arg(current_item_.filename));
                emit next();
                return;
            }
            QSsh::SftpJobId id=channel_->uploadFile(temp_file->fileName(), QDir(destination_.path()).absoluteFilePath(current_item_.filename), QSsh::SftpOverwriteExisting);
            sftp_ops_.insert(id,Copy);
            // store pointer to keep temp file around until op finishes
            temp_files_.insert(id,QSharedPointer<QTemporaryFile>(temp_file));
        }else{
            QSsh::SftpJobId id=channel_->uploadFile(path, QDir(destination_.path()).absoluteFilePath(current_item_.filename), QSsh::SftpOverwriteExisting);
            sftp_ops_.insert(id,Copy);
        }
    }
}

void RemoteExportWorker::checkDestinationDirectory_()
{
    QSsh::SftpJobId id=channel_->statFile(QDir(destination_.path()).absoluteFilePath(current_item_.directories.first()));
    sftp_ops_.insert(id,DirStat);
}

void RemoteExportWorker::createDirectory_()
{
    QSsh::SftpJobId id=channel_->createDirectory(QDir(destination_.path()).absoluteFilePath(current_item_.directories.first()));
    sftp_ops_.insert(id,MkDir);
}



void RemoteExportWorker::createLink_()
{
    bool is_open_ssh=true;
    QByteArray buffer(2048,0);
    ssize_t len=readlink(source_.absoluteFilePath(current_item_.filename).toLatin1().data(), buffer.data(), static_cast<size_t>(buffer.size()));
    if(len>0){
        QSsh::SftpJobId id;
        if(is_open_ssh){
            // Open SSH expects reverse order of arguments for symlinks
            id=channel_->createLink(QString(buffer),QDir(destination_.path()).absoluteFilePath(current_item_.filename));
        }else{
            id=channel_->createLink(QDir(destination_.path()).absoluteFilePath(current_item_.filename),QString(buffer));
        }
        sftp_ops_.insert(id,Link);
    }else{
        error_(QString("Error reading local symlink: %1").arg(current_item_.filename));
        next();
    }
}


void RemoteExportWorker::connected_()
{
    message_("Connected");
    channel_=ssh_connection_->createSftpChannel();
    connect(channel_.data(),&QSsh::SftpChannel::initialized,this,&RemoteExportWorker::initialized_);
    connect(channel_.data(),&QSsh::SftpChannel::initializationFailed,this,&RemoteExportWorker::initializationFailed_);
    connect(channel_.data(),&QSsh::SftpChannel::finished,this,&RemoteExportWorker::sftpOpFinished_);
    connect(channel_.data(),&QSsh::SftpChannel::fileInfoAvailable,this,&RemoteExportWorker::fileInfo_);
    channel_->initialize();
}

void RemoteExportWorker::connectionFailed_()
{
    error_("SSH connection failed");
}

void RemoteExportWorker::initialized_()
{
    message_("Initialized");
    processNext_();
}

void RemoteExportWorker::initializationFailed_()
{
    error_("SFTP channel inizialization failed");
}

void RemoteExportWorker::sftpOpFinished_(QSsh::SftpJobId job, const QString &err)
{
    //message_(QString("Finished job: %1").arg(job));
    if(!sftp_ops_.contains(job)){
        error_(QString("Unknown sftp job id encountered: %1").arg(job));
    }
    OpType op=sftp_ops_.take(job);
    switch (op) {
    case Copy:
        if(temp_files_.contains(job)){
            temp_files_.remove(job);
        }
        if(err!=""){
            error_(QString("Couldn't copy file: %1 (%2)").arg(QDir(destination_.path()).absoluteFilePath(current_item_.filename)).arg(err));
        }else{
            message_(QString("Copied file: %1").arg(QDir(destination_.path()).absoluteFilePath(current_item_.filename)));
        }
        next();
        break;
    case Link:
        if(err!=""){
            error_(QString("Couldn't create link: %1").arg(QDir(destination_.path()).absoluteFilePath(current_item_.filename)));
        }else{
            message_(QString("Created link: %1").arg(QDir(destination_.path()).absoluteFilePath(current_item_.filename)));
        }
        next();
        break;
    case LinkStat:
        if(err!=""){
            // link doesn't exist at destination and has to be created
            createLink_();
        }else{
            // nothing to do here as succesful LinkStat will be handled in fileInfo_
        }
        break;
    case MkDir:
        if(err!=""){
            QString dir_path=current_item_.directories.first();
            message_(QString("MkDir err: %1 (%2)").arg(QDir(destination_.path()).absoluteFilePath(dir_path)).arg(err));
            emit  next();
        }else{
            current_item_.directories.takeFirst();
            if(!current_item_.directories.empty()){
                checkDestinationDirectory_();
            }else{
                emit next();
            }
        }
        break;
    case DirStat:
        if(err!=""){
            // directory doesn't exist at destination and has to be created
            createDirectory_();
        }else{
            // nothing to do here as succesful DirStat will be handled in fileInfo_
        }
        break;
    case NoOp:
    default:
        if(err!=""){
            error_(err);
        }
        break;
    }
}


void RemoteExportWorker::fileInfo_(QSsh::SftpJobId job, const QList<QSsh::SftpFileInfo> &file_info_list)
{
    if(!sftp_ops_.contains(job)){
        error_(QString("Unknown sftp job id encountered: %1").arg(job));
    }
    OpType op=sftp_ops_.value(job);
    switch(op){
    case DirStat:
        // Directory path exists at destiantion. Emit error if it is not a directory
        if(file_info_list[0].type!=QSsh::FileTypeDirectory){
            error_(QString("Path %1 exists, but is not a directory.").arg(QDir(destination_.path()).absoluteFilePath(current_item_.directories.first())));
            current_item_=WorkItem();
            emit next();
        }else{
            current_item_.directories.takeFirst();
            if(!current_item_.directories.empty()){
                checkDestinationDirectory_();
            }else{
                emit next();
            }
        }
        break;
    case LinkStat:
        if(file_info_list[0].type==QSsh::FileTypeOther ||file_info_list[0].type==QSsh::FileTypeRegular){
            // Assume that the link already exists
        }else{
            error_(QString("Path %1 exists, but is not a link. Encountered filetype %2.").arg(QDir(destination_.path()).absoluteFilePath(current_item_.filename)).arg(file_info_list[0].type));
        }
        emit next();
        break;
    default:
        break;
    }

}





ParallelExporter::ParallelExporter(const QString &source, const SftpUrl &destination, const QStringList& images, int num_threads, QObject *parent):
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
        QStringList splitted_path=path.split("/",QString::SkipEmptyParts);
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
            QList<QString> dir_paths=directories[key].toList();
            qSort(dir_paths);
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

SftpUrl ParallelExporter::destination() const
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


