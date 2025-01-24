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

#ifndef PARALLELEXPORTER_H
#define PARALLELEXPORTER_H

#include <QObject>
#include <QAtomicInteger>
#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include <QSet>
#include <QByteArray>
#include <QRegularExpression>
#include <QTemporaryFile>
#include <QDir>
#include <QHash>
#include <QSharedPointer>
#include <QTimer>
#include <QAtomicInt>
#include <QUrl>
#include "sftpsession.h"

//fw decl
class ExportProgressDialog;

template <class T>
class ThreadSafeList{
protected:
    QList<T> list_;
public:
    typedef QPair<bool,T> result_pair;
    mutable QMutex mutex;
    ThreadSafeList():
        list_(),
        mutex()
    {}
    ThreadSafeList(const ThreadSafeList<T>& other):
        list_(),
        mutex()
    {
        QMutexLocker(other.mutex);
        list_=other.list_;
    }
    QList<T> list() const{
        QMutexLocker locker(&mutex);
        return list_;
    }
    result_pair checkEmptyAndTakeFirst(){
        QMutexLocker locker(&mutex);
        if(list_.empty()){
            return result_pair(true,T());
        }
        return result_pair(false,list_.takeFirst());
    }
    void append(const T &t){
        QMutexLocker locker(&mutex);
        list_.append(t);
    }
    void append(QList<T> &other){
        QMutexLocker locker(&mutex);
        list_.append(other);
    }
    int size() const{
        QMutexLocker locker(&mutex);
        return list_.size();
    }
    bool empty() const{
        QMutexLocker locker(&mutex);
        return list_.empty();
    }
    void clear(){
        QMutexLocker locker(&mutex);
        list_.clear();
    }
    result_pair checkEmptyAndFirst(){
        QMutexLocker locker(&mutex);
        if(list_.empty()){
            return result_pair(true,T());
        }
        return result_pair(false,list_.first());
    }
};
template <class T>
class ThreadSafeQueue: public ThreadSafeList<T>{
public:
    typename ThreadSafeList<T>::result_pair checkEmptyAndDequeue(){
        return ThreadSafeList<T>::checkEmptyAndTakeFirst();
    }
    void enqueue(const T &t){
        ThreadSafeList<T>::append(t);
    }
};

struct FileItem{
    FileItem(const QString& p="", bool f=true): path(p),filter(f){}
    QString path;
    bool filter;
};

struct ExportMessage{
    enum Type{
        ERROR,
        MESSAGE
    };
    int id;
    Type type;
    QString text;
    QDateTime timestamp;
    static QAtomicInteger<quint64> counter;
    quint64 counter_;
    ExportMessage(int mid,Type mtype,const QString mtext):
        id(mid),
        type(mtype),
        text(mtext),
        timestamp(QDateTime::currentDateTime()),
        counter_(++counter)
    {}
};


class ExportWorkerBase : public QObject
{
    Q_OBJECT
signals:
    void directoriesCreated();
    void finished();
    void nextFile();
public:
    ExportWorkerBase(int id, QSharedPointer<ThreadSafeQueue<FileItem> > queue, const QString &source, const QUrl &destination, const QStringList& exported_micrographs, QFileDevice::Permissions permissions);
    ~ExportWorkerBase();
    bool busy() const;
    QList<ExportMessage> messages();
    int id() const;
public slots:
    virtual void createDirectories(const QStringList& directories);
    virtual void copyFile();
protected:
    virtual void init_(){}
    virtual void copyFile_(const QString& path)=0;
    virtual void copyFilteredFile_(const QString& path, const QByteArray& data)=0;
    virtual void createDirectory_(const QString& path)=0;
    void error_(const QString& error);
    void message_(const QString& message);
    QByteArray filter_(const QString& source_path);
    QSharedPointer<ThreadSafeQueue<FileItem> > queue_;
    QDir source_;
    QUrl destination_;
    QStringList exported_micrographs_;
    QRegularExpression image_name_;
    ThreadSafeList<ExportMessage> message_buffer_;
    int id_;
    int busy_;
    QFileDevice::Permissions permissions_;
    QFileDevice::Permissions dir_permissions_;

};

class LocalExportWorker: public ExportWorkerBase{
    Q_OBJECT
public:
    LocalExportWorker(int id, QSharedPointer<ThreadSafeQueue<FileItem> > queue, const QString &source, const QUrl &destination, const QStringList& exported_micrographs, QFileDevice::Permissions permissions);
protected:
    virtual void copyFile_(const QString& path);
    virtual void copyFilteredFile_(const QString& path, const QByteArray& data);
    virtual void createDirectory_(const QString& path);
};

class RemoteExportWorker: public ExportWorkerBase{
    Q_OBJECT
public:
    RemoteExportWorker(int id, QSharedPointer<ThreadSafeQueue<FileItem> > queue, const QString &source, const QUrl &destination, const QStringList& exported_micrographs, QFileDevice::Permissions permissions, int msec_delay);
    virtual void init_();
    virtual void copyFile_(const QString& path);
    virtual void copyFilteredFile_(const QString& path, const QByteArray& data);
    virtual void createDirectory_(const QString& path);
    SFTPSession sftp_session_;
    int msec_delay_;

};


class ParallelExporter : public QObject
{
    Q_OBJECT
public:
    explicit ParallelExporter(const QString &source, const QUrl &destination,const QStringList& exported_micrographs,const QStringList& files,const QStringList& filtered_files, QFileDevice::Permissions permissions,int num_threads=1,QObject *parent = nullptr);
    ~ParallelExporter();
    QUrl destination() const;
    int numFiles() const;
signals:
    void createDirectories(const QStringList& directories);
    void copyFiles();
    void message(int files_left,const QList<ExportMessage>& m);
    void finished();
    void deleteThreads();
public slots:
    void cancel();
    void start();
    void getMessages();
    void directoriesCreated();
protected slots:
protected:
    QString source_;
    QUrl destination_;
    QStringList exported_micrographs_;
    int num_threads_;
    QSharedPointer<ThreadSafeQueue<FileItem> > queue_;
    QStringList directories_;
    bool started_;
    QList<ExportWorkerBase*> workers_;
    QTimer message_timer_;
    ExportProgressDialog* export_progress_dialog_;
private:
    Q_DISABLE_COPY(ParallelExporter)
};



#endif // PARALLELEXPORTER_H
