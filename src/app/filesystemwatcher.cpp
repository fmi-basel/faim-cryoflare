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

#include "filesystemwatcher.h"
#include "filesystemwatcherimpl.h"


FileSystemWatcher::FileSystemWatcher(QObject *parent) :
    QObject(parent),
    thread_(new QThread),
    impl_(new FileSystemWatcherImpl)
{
    init_impl();
}


FileSystemWatcher::FileSystemWatcher(const QStringList &paths, QObject *parent):
    QObject(parent),
    thread_(new QThread),
    impl_(new FileSystemWatcherImpl)
{
    init_impl();
    addPaths(paths);
}

FileSystemWatcher::~FileSystemWatcher()
{

}

void FileSystemWatcher::init_impl()
{
    connect(thread_, SIGNAL(started()), impl_, SLOT(start()));
    connect(thread_, SIGNAL(finished()), impl_, SLOT(deleteLater()));
    impl_->moveToThread(thread_);
    connect(impl_, SIGNAL(directoryChanged(const QString &)), this, SIGNAL(directoryChanged(const QString &)));
    connect(impl_, SIGNAL(fileChanged(const QString &)), this, SIGNAL(fileChanged(const QString &)));
    connect(this,&FileSystemWatcher::destroyed,impl_,&FileSystemWatcherImpl::deleteLater);
    connect(this,&FileSystemWatcher::destroyed,thread_,&QThread::deleteLater);
    thread_->start();
}

bool FileSystemWatcher::addPath(const QString &path)
{
    impl_->addPath(path);
    return true;
}

bool FileSystemWatcher::addPaths(const QStringList &paths)
{
    impl_->addPaths(paths);
    return true;
}

QStringList FileSystemWatcher::directories() const
{
    return impl_->directories();
}

QStringList FileSystemWatcher::files() const
{
    return impl_->files();
}

void FileSystemWatcher::removePath(const QString &path)
{
    impl_->removePath(path);
}

void FileSystemWatcher::removePaths(const QStringList &paths)
{
    impl_->removePaths(paths);
}

void FileSystemWatcher::removeAllPaths()
{
    impl_->removeAllPaths();
}

