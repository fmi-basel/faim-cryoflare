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
#include "filesystemwatcherimpl.h"

FileSystemWatcherImpl::FileSystemWatcherImpl(QObject *parent) :
    QObject(parent)
{
}

void FileSystemWatcherImpl::addPath(const QString &path)
{
    mutex.lock();
    QFileInfo finfo(path);
    if( finfo.exists()){
        if(finfo.isDir()){
            dirs_.append(path);
            dir_file_mod_times_[path]=QHash<QString,QDateTime>();
            foreach(QFileInfo info, QDir(path).entryInfoList(QDir::AllEntries|QDir::NoDotAndDotDot)){
                dir_file_mod_times_[path][info.canonicalFilePath()]=info.lastModified();
            }
        }else{
            files_.append(path);
            QFileInfo info(path);
            file_mod_times_[path]=info.lastModified();
        }
    }

    mutex.unlock();
}

void FileSystemWatcherImpl::addPaths(const QStringList &paths)
{
    foreach(QString path,paths){
        addPath(path);
    }
}

QStringList FileSystemWatcherImpl::directories() const
{
    mutex.lock();
    QStringList result=dirs_;
    mutex.unlock();
    return result;
}

QStringList FileSystemWatcherImpl::files() const
{
    mutex.lock();
    QStringList result=files_;
    mutex.unlock();
    return result;
}

void FileSystemWatcherImpl::removePath(const QString &path)
{
    mutex.lock();
    if(dirs_.removeOne(path)){
        dir_file_mod_times_.remove(path);
    }else{
        if(files_.removeOne(path)){
            file_mod_times_.remove(path);
        }
    }
    mutex.unlock();
}

void FileSystemWatcherImpl::removePaths(const QStringList &paths)
{
    foreach(QString path,paths){
        removePath(path);
    }
}

void FileSystemWatcherImpl::removeAllPaths()
{
    mutex.lock();
    dirs_.clear();
    dir_file_mod_times_.clear();
    files_.clear();
    file_mod_times_.clear();
    mutex.unlock();
}

void FileSystemWatcherImpl::update(){
    mutex.lock();
    foreach(QString path,dirs_){
        QFileInfoList entry_list=QDir(path).entryInfoList(QDir::AllEntries|QDir::NoDotAndDotDot);
        if(entry_list.size()!=dir_file_mod_times_.value(path).size()){
            emit directoryChanged(path);
            dir_file_mod_times_[path]=QHash<QString,QDateTime>();
            foreach(QFileInfo info, entry_list){
                dir_file_mod_times_[path][info.canonicalFilePath()]=info.lastModified();
            }
        }else{
            foreach(QFileInfo info, entry_list){
                if(info.lastModified()!=dir_file_mod_times_.value(path).value(info.canonicalFilePath())){
                    emit directoryChanged(path);
                    dir_file_mod_times_[path][info.canonicalFilePath()]=info.lastModified();
                    break;
                }
            }
        }

    }
    foreach(QString path,files_){
        QFileInfo info(path);
        if(file_mod_times_.value(path)!=info.lastModified()){
            emit fileChanged(path);
            file_mod_times_[path]=info.lastModified();
        }
    }
    mutex.unlock();
}
