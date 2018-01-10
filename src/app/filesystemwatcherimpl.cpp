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
