//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2017-2019 by the CryoFLARE Authors
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

#ifndef FILESYSTEMWATCHER_H
#define FILESYSTEMWATCHER_H

#include <QObject>
#include <QThread>
#include <QFileInfo>


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
public slots:
    void start();
    void stop();
signals:
    void directoryChanged(const QString & path, const QList<QFileInfo> & changed_files);
    void fileChanged(const QString & path);
    void startImpl_(QPrivateSignal);
    void stopImpl(QPrivateSignal);
public slots:
protected:
    void init_impl();
    QThread *thread_;
    FileSystemWatcherImpl* impl_;

};

#endif // FILESYSTEMWATCHER_H
