//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2020 by the CryoFLARE Authors
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
#include "datafolderwatcher.h"
#include <QMutableListIterator>
#include <QtConcurrent/QtConcurrentMap>
#include <QDir>

void mergeParsedData(ParsedData &result, const ParsedData &intermediate)
{
    result.micrographs.append(intermediate.micrographs);
    result.foil_holes.append(intermediate.foil_holes);
    result.grid_squares.append(intermediate.grid_squares);
}

DataFolderWatcher::DataFolderWatcher(QObject *parent):
    QObject(parent),
    root_folder_(),
    watcher_(new FileSystemWatcher()),
    project_dir_(),
    movie_dir_(),
    watched_dirs_(),
    future_watchers_()
{
    connect(watcher_.data(),&FileSystemWatcher::directoryChanged,this,&DataFolderWatcher::onDirChanged);
}

void DataFolderWatcher::setRootFolder(const FolderNode &node)
{
    root_folder_=node;
}

void DataFolderWatcher::setProjectDir(const QString &project_dir)
{
    if(project_dir_!=project_dir){
        project_dir_=project_dir;
        watcher_->removeAllPaths();
        watched_dirs_.clear();
        watcher_->addPath(project_dir_);
        watched_dirs_.append(project_dir_);
    }
}

void DataFolderWatcher::setMovieDir(const QString &movie_dir)
{
    movie_dir_=movie_dir;
}

void DataFolderWatcher::start()
{
    watcher_->start();
}

void DataFolderWatcher::stop()
{
    watcher_->stop();
}

void DataFolderWatcher::onDirChanged(const QString &path, QList<QFileInfo> changed_files)
{
    QString relative_path=QDir(project_dir_).relativeFilePath(path);
    QStringList dir_list;
    if(relative_path!="."){
        dir_list=relative_path.split("/");
    }
    FolderNode current_node=root_folder_;
    while (!dir_list.empty()) {
        QString dir_name=dir_list.first();
        foreach(FolderNode node,current_node.children){
            if(dir_name.contains(node.pattern)){
                current_node=node;
                dir_list.takeFirst();
                break;
            }
        }
        if(!dir_list.empty() && dir_name==dir_list.first()){
            break;
        }
    }
    if(! dir_list.empty()){
        // directory doesn't match any pattern
        return;
    }
    QMutableListIterator<QFileInfo> it(changed_files);
    while (it.hasNext()) {
        QFileInfo info=it.next();
        if (info.isDir()){
            foreach(FolderNode child,current_node.children){
                if(info.fileName().contains(child.pattern)){
                    QString abs_path=info.absoluteFilePath();
                    if(!watched_dirs_.contains(abs_path)){
			//qDebug() << QDateTime::currentDateTime() << "adding new directory to watch: " << abs_path;
                        watched_dirs_.append(abs_path);
                        watcher_->addPath(abs_path);
                    }
                }
            }
            it.remove();
        }
    }
    foreach(ReaderPair pair, current_node.file_readers){
        QList<QFileInfo> matching_files;
        QMutableListIterator<QFileInfo> it(changed_files);
        while (it.hasNext()) {
            QFileInfo info=it.next();
            if (info.fileName().contains(pair.first)){
                matching_files.append(info);
                it.remove();
            }
        }
        if(! matching_files.empty()){
            std::function<ParsedData(const QFileInfo&)> reader = [project_dir= project_dir_, movie_dir=movie_dir_, func=pair.second](const QFileInfo& info){ return func(info,project_dir,movie_dir); };
            //qDebug() << QDateTime::currentDateTime() << "parsing " << matching_files.size() << " files in directory " << path << " matching pattern " << pair.first.pattern();
            QFutureWatcher<ParsedData>* watcher=new  QFutureWatcher<ParsedData>();
            watcher->setFuture(QtConcurrent::mappedReduced(matching_files,reader,mergeParsedData,QtConcurrent::OrderedReduce));
            connect(watcher,&QFutureWatcher<ParsedData>::finished, this,[=]() {fileReadFinished(watcher);});
            future_watchers_.append(watcher);
        }
    }
}

void DataFolderWatcher::fileReadFinished(QFutureWatcher<ParsedData> *watcher)
{
    ParsedData parsed_data=watcher->result();
    future_watchers_.removeAll(watcher);
    delete watcher;
    emit newDataAvailable(parsed_data);
}
