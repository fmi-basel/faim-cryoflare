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

#ifndef PARALLELEXPORTER_H
#define PARALLELEXPORTER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QQueue>
#include <QMutex>
#include <QSet>
#include <QByteArray>

//fw decl
class QProgressDialog;

class Worker : public QObject
{
    Q_OBJECT
public:
    Worker(const QString &source, const QString &destination, QQueue<QSet<QString> > *queue, QMutex *mutex, const QString &mode, const QString &script, QObject *parent = 0);
    ~Worker();

public slots:
    void process();

signals:
    void nextImage();
    void finished(const QByteArray& output, const QByteArray& error);
protected:
    QString source_;
    QString destination_;
    QQueue<QSet<QString> > *queue_;
    QMutex *mutex_;
    QString mode_;
    QString script_;
    QByteArray output_;
    QByteArray error_;
};

class ParallelExporter : public QObject
{
    Q_OBJECT
public:
    explicit ParallelExporter(QObject *parent = 0);
    void exportImages(const QString &source, const QString &destination, QQueue<QSet<QString> > &image_list, int num_processes=1, const QString& mode=QString("copy"), const QString& custom_script=QString(""), const QString& pre_script=QString(""), const QString& post_script=QString(""),const QStringList& image_names=QStringList());
signals:

public slots:
    void updateProgress();
    void cancel();
    void runPost(const QByteArray& output, const QByteArray& error);
protected:
    QQueue<QSet<QString> > queue_;
    int num_tasks_;
    int num_tasks_done_;
    QProgressDialog* dialog_;
    QMutex mutex_;
    int num_threads_;
    QString post_script_;
    QStringList post_arguments_;
    QByteArray output_;
    QByteArray error_;
};

#endif // PARALLELEXPORTER_H
