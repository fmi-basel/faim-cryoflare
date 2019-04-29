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
#include <QMutableMapIterator>
#include "filesystemwatcherimpl.h"

FileSystemWatcherImpl::FileSystemWatcherImpl(QObject *parent) :
    QObject(parent),
    timer_(new QTimer(this)),
    files_(),
    dirs_(),
    mod_times_(),
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
        if(finfo.isDir()){
            QMap<QString,QDateTime> mod_times_local;
            QFileInfoList child_items=QDir(path).entryInfoList(QDir::AllEntries|QDir::NoDotAndDotDot);
            foreach(QFileInfo info, child_items){
                mod_times_local[info.canonicalFilePath()]=info.lastModified();
            }
            if(child_items.size()>0){
                emit_signal=true;
            }
            QMutexLocker locker(&mutex);
            dirs_.append(path);
            dirs_.sort();
            foreach(QString key, mod_times_local.keys()){
                mod_times_[key]=mod_times_local.value(key);
            }
        }else{
            QFileInfo info(path);
            QDateTime modified=info.lastModified();
            QMutexLocker locker(&mutex);
            files_.append(path);
            files_.sort();
            mod_times_[path]=modified;
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
        QMutableMapIterator<QString, QDateTime> it(mod_times_);
        while (it.hasNext()) {
            it.next();
            if (it.key().startsWith(path) && ! files_.contains(it.key())){
                it.remove();
            }
        }
    }else{
        if(files_.removeOne(path)){
            mod_times_.remove(path);
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
    files_.clear();
    mod_times_.clear();
}

void FileSystemWatcherImpl::start()
{
    timer_->start();
}

void FileSystemWatcherImpl::update(){
    QStringList emit_dirs;
    QStringList emit_files;
    mutex.lock();
    QStringList dirs_local=dirs_;
    QStringList files_local=files_;
    QMap<QString,QDateTime> mod_times_local=mod_times_;
    QMap<QString,QDateTime> mod_times_updates;
    mutex.unlock();
    // make sure that mutex is ulocked for filesystem access, as filesystem might potentially hang
    foreach(QString path,dirs_local){
        QFileInfoList entry_list=QDir(path).entryInfoList(QDir::AllEntries|QDir::NoDotAndDotDot);
        bool has_changes=false;
        foreach(QFileInfo info, entry_list){
            QString child_path=info.canonicalFilePath();
            QDateTime last_modified=info.lastModified();
            if(! mod_times_local.contains(child_path) || mod_times_local.value(child_path)!=last_modified){
                has_changes=true;
                mod_times_updates[child_path]=last_modified;
            }
        }
        if(has_changes){
            emit_dirs<<path;
        }
    }
    foreach(QString path,files_local){
        QFileInfo info(path);
        if(mod_times_local.value(path)!=info.lastModified()){
            emit_files<<path;
            mod_times_updates[path]=info.lastModified();
        }
    }
    mutex.lock();
    //update from local values as long as lists were not changed in between
    if(dirs_ == dirs_local && files_ ==files_local){
        foreach(QString key, mod_times_updates.keys()){
            mod_times_[key]=mod_times_updates.value(key);
        }
    }else{
        emit_dirs.clear();
        emit_files.clear();
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
