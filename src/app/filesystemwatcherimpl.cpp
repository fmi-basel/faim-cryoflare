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
#include <QMutexLocker>
#include "filesystemwatcherimpl.h"

FileSystemWatcherImpl::FileSystemWatcherImpl(QObject *parent) :
    QObject(parent),
    timer_(new QTimer(this)),
    files_(),
    dirs_(),
    file_mod_times_(),
    dir_file_mod_times_(),
    mutex()
{
    timer_->setInterval(5000);
    timer_->setSingleShot(true);
    connect(timer_, &QTimer::timeout, this, &FileSystemWatcherImpl::update);
}

void FileSystemWatcherImpl::addPath(const QString &path)
{
    QFileInfo finfo(path);
    bool emit_signal=false;
    if( finfo.exists()){
        QMutexLocker locker(&mutex);
        if(finfo.isDir()){
            dirs_.append(path);
            dir_file_mod_times_[path]=QHash<QString,QDateTime>();
            QFileInfoList child_items=QDir(path).entryInfoList(QDir::AllEntries|QDir::NoDotAndDotDot);
            foreach(QFileInfo info, child_items){
                dir_file_mod_times_[path][info.canonicalFilePath()]=info.lastModified();
            }
            if(child_items.size()>0){
                emit_signal=true;
            }
        }else{
            files_.append(path);
            QFileInfo info(path);
            file_mod_times_[path]=info.lastModified();
        }
    }
    //emit only after unlocking mutex to avoid deadlocks
    if(emit_signal){
        emit directoryChanged(path);
    }
}

void FileSystemWatcherImpl::addPaths(const QStringList &paths)
{
    foreach(QString path,paths){
        addPath(path);
    }
}

QStringList FileSystemWatcherImpl::directories() const
{
    QMutexLocker locker(&mutex);
    QStringList result=dirs_;
    return result;
}

QStringList FileSystemWatcherImpl::files() const
{
    QMutexLocker locker(&mutex);
    QStringList result=files_;
    return result;
}

void FileSystemWatcherImpl::removePath(const QString &path)
{
    QMutexLocker locker(&mutex);
    if(dirs_.removeOne(path)){
        dir_file_mod_times_.remove(path);
    }else{
        if(files_.removeOne(path)){
            file_mod_times_.remove(path);
        }
    }
}

void FileSystemWatcherImpl::removePaths(const QStringList &paths)
{
    foreach(QString path,paths){
        removePath(path);
    }
}

void FileSystemWatcherImpl::removeAllPaths()
{
    QMutexLocker locker(&mutex);
    dirs_.clear();
    dir_file_mod_times_.clear();
    files_.clear();
    file_mod_times_.clear();
}

void FileSystemWatcherImpl::start()
{
    timer_->start();
}

void FileSystemWatcherImpl::update(){
    QStringList emit_dirs;
    QStringList emit_files;
    mutex.lock();
    foreach(QString path,dirs_){
        QFileInfoList entry_list=QDir(path).entryInfoList(QDir::AllEntries|QDir::NoDotAndDotDot);
        if(entry_list.size()!=dir_file_mod_times_.value(path).size()){
            emit_dirs<<path;
            dir_file_mod_times_[path]=QHash<QString,QDateTime>();
            foreach(QFileInfo info, entry_list){
                dir_file_mod_times_[path][info.canonicalFilePath()]=info.lastModified();
            }
        }else{
            foreach(QFileInfo info, entry_list){
                if(info.lastModified()!=dir_file_mod_times_.value(path).value(info.canonicalFilePath())){
                    emit_dirs << path;
                    dir_file_mod_times_[path][info.canonicalFilePath()]=info.lastModified();
                    break;
                }
            }
        }

    }
    foreach(QString path,files_){
        QFileInfo info(path);
        if(file_mod_times_.value(path)!=info.lastModified()){
            emit_files<<path;
            file_mod_times_[path]=info.lastModified();
        }
    }
    mutex.unlock();
    timer_->start();
    //emit only after unlocking mutex to avoid deadlocks
    foreach(QString path,emit_dirs){
        emit directoryChanged(path);
    }
    foreach(QString path, emit_files){
        emit fileChanged(path);
    }
}
