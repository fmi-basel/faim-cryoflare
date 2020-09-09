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
#ifndef FOLDERWATCHER_H
#define FOLDERWATCHER_H

#include "dataptr.h"
#include "filesystemwatcher.h"
#include <QObject>
#include <QFileInfo>
#include <QRegularExpression>
#include <QFutureWatcher>


struct ParsedData{
    QList<Data> grid_squares;
    QList<Data> foil_holes;
    QList<Data> micrographs;
};
typedef ParsedData (*FileReaderPtr)(const QFileInfo&,const QString&, const QString&);
typedef QPair<QRegularExpression,FileReaderPtr> ReaderPair;
struct FolderNode {
    QRegularExpression pattern;
    QList<FolderNode> children;
    QList<ReaderPair> file_readers;
};


class DataFolderWatcher : public QObject
{
    Q_OBJECT
public:
    explicit DataFolderWatcher(QObject *parent = nullptr);
    void setRootFolder(const FolderNode& node);

signals:
    void newDataAvailable(const ParsedData& data);
public slots:
    void setProjectDir(const QString& project_dir);
    void setMovieDir(const QString& movie_dir);
    void start();
    void stop();
protected slots:
    virtual void onDirChanged(const QString & path, QList<QFileInfo> changed_files);
    void fileReadFinished(QFutureWatcher<ParsedData> * watcher);
protected:
    FolderNode root_folder_;
    QScopedPointer<FileSystemWatcher> watcher_;
    QString project_dir_;
    QString movie_dir_;
    QStringList watched_dirs_;
    QList<QFutureWatcher<ParsedData> *> future_watchers_;
};

#endif // FOLDERWATCHER_H
