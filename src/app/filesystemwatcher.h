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

#ifndef FILESYSTEMWATCHER_H
#define FILESYSTEMWATCHER_H

#include <QObject>
#include <QTimer>
#include <QThread>


//fw decl
class FileSystemWatcherImpl;
class FileSystemWatcher : public QObject
{
    Q_OBJECT
public:
    explicit FileSystemWatcher(QObject *parent = nullptr);
    FileSystemWatcher(const QStringList & paths, QObject * parent = nullptr);
    ~FileSystemWatcher();
    bool addPath(const QString & path);
    bool addPaths(const QStringList & paths);
    QStringList directories() const;
    QStringList files() const;
    void removePath(const QString & path);
    void removePaths(const QStringList & paths);
    void removeAllPaths();

signals:
    void directoryChanged(const QString & path);
    void fileChanged(const QString & path);
public slots:
protected:
    void init_impl();
    QTimer *timer_;
    QThread *thread_;
    FileSystemWatcherImpl* impl_;

};

#endif // FILESYSTEMWATCHER_H
