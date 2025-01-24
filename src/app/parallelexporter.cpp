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
#include <algorithm>
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

QAtomicInteger<quint64> ExportMessage::counter=0;

bool operator<(const ExportMessage& lhs, const ExportMessage& rhs){
    return lhs.timestamp==rhs.timestamp ? lhs.counter_<rhs.counter_ : lhs.timestamp<rhs.timestamp;
}


ExportWorkerBase::ExportWorkerBase(int id, QSharedPointer<ThreadSafeQueue<FileItem> > queue, const QString &source, const QUrl &destination, const QStringList &export_micrographs, QFileDevice::Permissions permissions):
    QObject(),
    queue_(queue),
    source_(source),
    destination_(destination),
    exported_micrographs_(export_micrographs),
    image_name_("\\d{8}_\\d{6}"),
    message_buffer_(),
    id_(id),
    busy_(false),
    permissions_(permissions),
    dir_permissions_(permissions)
{
    if(permissions && QFileDevice::ReadOwner){
        dir_permissions_|=QFileDevice::ExeOwner;
    }
    if(permissions && QFileDevice::ReadGroup){
        dir_permissions_|=QFileDevice::ExeGroup;
    }
    if(permissions && QFileDevice::ReadOther){
        dir_permissions_|=QFileDevice::ExeOther;
    }
    // queued connection here to allow signal to reach main event loop as well
    connect(this,&ExportWorkerBase::nextFile,this,&ExportWorkerBase::copyFile,Qt::QueuedConnection);

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

void ExportWorkerBase::error_(const QString &error)
{
    message_buffer_.append(ExportMessage(id_,ExportMessage::ERROR,error));
}

void ExportWorkerBase::message_(const QString &message)
{
    message_buffer_.append(ExportMessage(id_,ExportMessage::MESSAGE,message));
}

QByteArray ExportWorkerBase::filter_(const QString &source_path)
{
    QByteArray filtered_data("");
    QTextStream out_stream(&filtered_data);
    QFile file(source_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        error_(QString("Couldn't read: %1").arg(source_path));
        return QByteArray();
    }
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        QRegularExpressionMatch match = image_name_.match(line);
        if(match.hasMatch()){
            if(exported_micrographs_.contains(match.captured(0))){
                out_stream<<line;
            }
        }else{
            out_stream<<line;
        }
        out_stream.flush();
    }
    return filtered_data;
}

int ExportWorkerBase::id() const
{
    return id_;
}

void ExportWorkerBase::createDirectories(const QStringList& directories)
{
    init_();
    foreach(QString dirname, directories){
        QString abs_dir_name=QDir(destination_.path()).absoluteFilePath(dirname);
        createDirectory_(abs_dir_name);
    }
    emit directoriesCreated();
}

void ExportWorkerBase::copyFile()
{
    init_();
    ThreadSafeQueue<FileItem>::result_pair result=queue_->checkEmptyAndDequeue();
    if(result.first){
        busy_=false;
        emit finished();
        return;
    }
    FileItem item=result.second;
    QString path=source_.absoluteFilePath(item.path);
    if(!QFileInfo(path).exists()){
        error_(QString("Skipping missing file: %1").arg(item.path));
    }else{
        if(item.filter){
            QByteArray data=filter_(path);
            if(data.isNull()){
            }else{
                copyFilteredFile_(item.path,data);
            }
        }else{
            copyFile_(item.path);
        }
    }
    emit nextFile();
}

LocalExportWorker::LocalExportWorker(int id, QSharedPointer<ThreadSafeQueue<FileItem> > queue, const QString &source, const QUrl &destination, const QStringList &exported_micrographs, QFileDevice::Permissions permissions):
    ExportWorkerBase(id,queue,source,destination,exported_micrographs,permissions)
{
}

void LocalExportWorker::copyFile_(const QString& path)
{
    QString abs_source_path=source_.absoluteFilePath(path);
    QString destination_file(QDir(destination_.path()).absoluteFilePath(path));
    if(QFile::exists(destination_file)){
        QFile::remove(destination_file);
    }
    QFile source_file(path);
    if(!source_file.copy(destination_file)){
        error_("Error copying file: "+path);
    }else{
        QFile(destination_file).setPermissions(permissions_);
        message_("Copied: "+path);
    }
}
void LocalExportWorker::copyFilteredFile_(const QString& path, const QByteArray& data)
{
    QString destination_file(QDir(destination_.path()).absoluteFilePath(path));
    QFile dest(destination_file);
    if (!dest.open(QIODevice::WriteOnly | QIODevice::Text)){
        error_("Error filering and copying file: "+path);
    }else{
        dest.write(data);
        dest.setPermissions(permissions_);
        message_("Copied and filtered: "+path);
    }
}

void LocalExportWorker::createDirectory_(const QString& directory)
{
    QDir dir(directory);
    if(!dir.exists()){
        QString dirname=dir.dirName();
        dir.cdUp();
        if(!dir.mkpath(dirname)){
            error_("Error creating directory: "+dirname);
        }
        QFile(directory).setPermissions(dir_permissions_);
    }
}


RemoteExportWorker::RemoteExportWorker(int id, QSharedPointer<ThreadSafeQueue<FileItem> > queue, const QString &source, const QUrl &destination, const QStringList &exported_micrographs, QFileDevice::Permissions permissions, int msec_delay):
    ExportWorkerBase(id,queue,source,destination,exported_micrographs,permissions),
    sftp_session_(),
    msec_delay_(msec_delay)
{
}


void RemoteExportWorker::init_()
{
    if(!busy()){
        if(!sftp_session_.isValid()){
            QThread::msleep(msec_delay_);
            message_("Initializing connection");
            if(!sftp_session_.connect(destination_)){
                error_("SFTP session inizialization failed");
                return;
            }
        }
    }
    busy_=true;
}

void RemoteExportWorker::copyFile_(const QString& path)
{
    QString abs_source_path=source_.absoluteFilePath(path);
    if(QFileInfo(abs_source_path).isSymLink()){
        QByteArray buffer(2048,0);
        ssize_t len=readlink(source_.absoluteFilePath(path).toLatin1().data(), buffer.data(), static_cast<size_t>(buffer.size()));
        if(len>0){
            QString target=QString::fromLatin1(buffer.data(),len);
            sftp_session_.createLink(QString(buffer),QDir(destination_.path()).absoluteFilePath(path));
        }else{
            error_(QString("Error reading local symlink: %1").arg(path));
        }
    }else{
        if(sftp_session_.writeFile(abs_source_path, QDir(destination_.path()).absoluteFilePath(path),permissions_)){
            message_(QString("Copied file: %1").arg(QDir(destination_.path()).absoluteFilePath(path)));
        }else{
            error_(QString("Couldn't copy file to: %1 (error %2, %3)").arg(QDir(destination_.path()).absoluteFilePath(path)).arg(sftp_session_.getError()).arg(sftp_session_.getErrorString()));
        }
    }
}

void RemoteExportWorker::copyFilteredFile_(const QString& path, const QByteArray& data)
{
    QDataStream data_stream(data);
    if(sftp_session_.writeFile(data_stream, QDir(destination_.path()).absoluteFilePath(path),permissions_)){
        message_(QString("Copied and filtered file: %1").arg(QDir(destination_.path()).absoluteFilePath(path)));
    }else{
        error_(QString("Couldn't copy and filter file to: %1 (error %2, %3)").arg(QDir(destination_.path()).absoluteFilePath(path)).arg(sftp_session_.getError()).arg(sftp_session_.getErrorString()));
    }
}

void RemoteExportWorker::createDirectory_(const QString& directory)
{
    if(!sftp_session_.isDir(directory)){
        message_(QString("Creating directory: %1").arg(directory));
        if(!sftp_session_.mkDir(directory,dir_permissions_)){
            error_(QString("Couldn't create directory: %1 (error %2)").arg(directory).arg(sftp_session_.getError()));
        }

    }else{
        message_(QString("Using existing directory: %1").arg(directory));
    }
}

ParallelExporter::ParallelExporter(const QString &source, const QUrl &destination, const QStringList& exported_micrographs, const QStringList &files, const QStringList &filtered_files, QFileDevice::Permissions permissions, int num_threads, QObject *parent):
    QObject(parent),
    source_(source),
    destination_(destination),
    exported_micrographs_(exported_micrographs),
    num_threads_(num_threads),
    queue_(new ThreadSafeQueue<FileItem>()),
    directories_(),
    started_(false),
    workers_(),
    message_timer_(),
    export_progress_dialog_(new ExportProgressDialog())
{
    QList<FileItem> file_items;
    //generate directories to create at destination
    QHash<QString,QSet<QString>> directory_hash;
    foreach(QString f,files+filtered_files){
        QString path=QFileInfo(f).path();
        if(path=="."){
            continue;
        }
        QStringList splitted_path=path.split("/",Qt::SkipEmptyParts);
        if(!directory_hash.contains(splitted_path[0])){
            directory_hash.insert(splitted_path[0],QSet<QString>());
        }
        for(int i=1;i<=splitted_path.size();++i){
            directory_hash[splitted_path[0]].insert(splitted_path.mid(0,i).join("/"));
        }
    }
    if(!directory_hash.empty()){
        foreach(QString key,directory_hash.keys()){
            QStringList dir_paths=directory_hash[key].values();
            std::sort(dir_paths.begin(),dir_paths.end());
            directories_.append(dir_paths);
        }
    }
    // create file queue
    foreach(QString f,files){
        file_items.append(FileItem(f,false));
    }
    foreach(QString f,filtered_files){
        file_items.append(FileItem(f,true));
    }
    queue_->append(file_items);
    //setup worker threads
    for(int i=0;i<num_threads_ ;++i){
        QThread* thread = new QThread();
        ExportWorkerBase* w;
        if(destination.isLocalFile()){
            w=new LocalExportWorker(i+1, queue_,source, destination, exported_micrographs,permissions);
        }else{
            w=new RemoteExportWorker(i+1, queue_,source, destination, exported_micrographs,permissions, i*20);
        }
        workers_.append(w);
        w->moveToThread(thread);
        if(i==0){
            connect(this,&ParallelExporter::createDirectories,w,&ExportWorkerBase::createDirectories);
            connect(w,&ExportWorkerBase::directoriesCreated,this,&ParallelExporter::directoriesCreated);
        }
        connect(this,&ParallelExporter::copyFiles,w,&ExportWorkerBase::copyFile);
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
    emit finished();
}

void ParallelExporter::start()
{
    started_=true;
    export_progress_dialog_->start(destination().toString(QUrl::RemovePassword),numFiles());
    emit createDirectories(directories_);
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
    std::sort(messages.begin(),messages.end());
    export_progress_dialog_->update(messages,left+n_busy);
    if(left==0 && n_busy==0){
        export_progress_dialog_->finish();
        emit finished();
    }
}

void ParallelExporter::directoriesCreated()
{
    emit copyFiles();
}


