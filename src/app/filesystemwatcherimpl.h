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

#ifndef FILESYSTEMWATCHERIMPL_H
#define FILESYSTEMWATCHERIMPL_H

#include <QObject>
#include <QHash>
#include <QMap>
#include <QDir>
#include <QMutex>
#include <QDateTime>
#include <QTimer>


class FileSystemWatcherImpl : public QObject
{
    Q_OBJECT
public:
    explicit FileSystemWatcherImpl(QObject *parent = nullptr);

    void addPath(const QString & path);

    void addPaths(const QStringList & paths);

    QStringList directories() const;

    QStringList files() const;

    void removePath(const QString & path);

    void removePaths(const QStringList & paths);
    void removeAllPaths();

signals:
    void directoryChanged(const QString & path, const QList<QFileInfo> & changed_files);
    void fileChanged(const QString & path);

public slots:
    void start();
    void stop();
    void update();

protected:
    QTimer *timer_;
    QStringList files_;
    QStringList dirs_;
    QMap<QString,QDateTime> mod_times_;
    mutable QMutex mutex;
    bool running_;
};

#endif // FILESYSTEMWATCHERIMPL_H
