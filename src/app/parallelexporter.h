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
#include <QWaitCondition>
#include <QSet>
#include <QByteArray>
#include <QRegularExpression>
#include <QTemporaryFile>
#include <QUrl>
#include <QDir>
#include <QHash>
#include <QSharedPointer>
#include <QTimer>
#include <QAtomicInt>
#include "../external/qssh/sshconnection.h"
#include "../external/qssh/sftpchannel.h"

template <class T>
class ThreadSafeList{
protected:
    QList<T> list_;
public:
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
    T takeFirst(){
        QMutexLocker locker(&mutex);
        return list_.takeFirst();
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
};
template <class T>
class ThreadSafeQueue: public ThreadSafeList<T>{
public:
    T dequeue(){
        return ThreadSafeList<T>::takeFirst();
    }
    void enqueue(const T &t){
        ThreadSafeList<T>::append(t);
    }
};


class Barrier{
public:
    Barrier(int n_threads): mutex_(),wait_condition_(), n_(n_threads-1){}
    void wait(){
        mutex_.lock();
        if(n_==0){
            wait_condition_.wakeAll();
            mutex_.unlock();
        }else{
            --n_;
            wait_condition_.wait(&mutex_);
            ++n_;
            mutex_.unlock();
        }
    }
protected:
    QMutex mutex_;
    QWaitCondition wait_condition_;
    int n_;
};

struct WorkItem{
    enum ItemType {
        BARRIER,FILE,DIRECTORY,INVALID
    };
    WorkItem(ItemType t=INVALID): directories(),filename(),type(t),filter(false){}
    QStringList directories;
    QString filename;
    ItemType type;
    bool filter;
    static WorkItem createFile(const QString& f, bool filt){
        WorkItem item(FILE);
        item.filename=f;
        item.filter=filt;
        return item;
    }
    static WorkItem createDirectory(const QStringList& dirs){
        WorkItem item(DIRECTORY);
        item.directories=dirs;
        return item;
    }
    static WorkItem createBarrier(){
        return WorkItem(BARRIER);
    }
};

struct ExportMessage{
    enum Type{
        ERROR,
        MESSAGE
    };
    int id;
    Type type;
    QString text;
    ExportMessage(int mid,Type mtype,const QString mtext):
        id(mid),
        type(mtype),
        text(mtext)
    {}
};

class ExportWorkerBase : public QObject
{
    Q_OBJECT
signals:
    void next();
public:

    ExportWorkerBase(int id, QSharedPointer<ThreadSafeQueue<WorkItem> > queue, const QString &source, const QUrl &destination, const QStringList& images, Barrier& b);
    ~ExportWorkerBase();
    bool busy() const;
    QList<ExportMessage> messages();
    int id() const;

public slots:
    void startExport();
protected:
    virtual void startImpl_()=0;
    void error_(const QString& error);
    void message_(const QString& message);
    virtual void processNextImpl_()=0;
    QTemporaryFile* filter_(const QString& source_path) const;
    QSharedPointer<ThreadSafeQueue<WorkItem> > queue_;
    QDir source_;
    QUrl destination_;
    QStringList images_;
    QRegularExpression image_name_;
    ThreadSafeList<ExportMessage> message_buffer_;
    Barrier& barrier_;
    int id_;
    int busy_;
protected slots:
    void processNext_();

};

class LocalExportWorker: public ExportWorkerBase{
    Q_OBJECT
public:
    LocalExportWorker(int id, QSharedPointer<ThreadSafeQueue<WorkItem> > queue, const QString &source, const QUrl &destination, const QStringList& images, Barrier& b);
protected:
    virtual void startImpl_();
    virtual void processNextImpl_();
    virtual void copyFile_(const WorkItem& item);
    virtual void createDirectory_(const WorkItem& item);
};

class RemoteExportWorker: public ExportWorkerBase{
    Q_OBJECT
public:
    enum OpType{
        NoOp,Stat,MkDir,Copy,DirStat,LinkStat,Link
    };

    RemoteExportWorker(int id, QSharedPointer<ThreadSafeQueue<WorkItem> > queue, const QString &source, const QUrl &destination, const QStringList& images, Barrier& b);
protected:
    virtual void startImpl_();
    virtual void processNextImpl_();
    virtual void copyFile_();
    virtual void checkDestinationDirectory_();
    virtual void createDirectory_();
    virtual void createLink_();
    QSsh::SshConnectionParameters connection_parameters_;
    QSsh::SshConnection* ssh_connection_;
    QSsh::SftpChannel::Ptr channel_;
    QHash<QSsh::SftpJobId,OpType> sftp_ops_;
    WorkItem current_item_;
    QHash<QSsh::SftpJobId,QSharedPointer<QTemporaryFile> > temp_files_;

protected slots:
    void connected_();
    void connectionFailed_();
    void initialized_();
    void initializationFailed_();
    void sftpOpFinished_(QSsh::SftpJobId job, const QString &err);
    void fileInfo_(QSsh::SftpJobId job, const QList<QSsh::SftpFileInfo> &fileInfoList);
};


class ParallelExporter : public QObject
{
    Q_OBJECT
public:
    explicit ParallelExporter(const QString &source, const QUrl &destination,const QStringList& images, int num_threads=1,QObject *parent = nullptr);
    ~ParallelExporter();
    void addImages(const QStringList& files, bool filter);
    QUrl destination() const;
    int numFiles() const;
signals:
    void newFiles();
    void message(int files_left,const QList<ExportMessage>& m);
    void finished();
    void deleteThreads();
public slots:
    void cancel();
    void start();
    void getMessages();
protected slots:
protected:
    QString source_;
    QUrl destination_;
    QStringList images_;
    int num_threads_;
    QSharedPointer<ThreadSafeQueue<WorkItem> > queue_;
    bool started_;
    QList<ExportWorkerBase*> workers_;
    QTimer message_timer_;
    Barrier barrier_;
private:
    Q_DISABLE_COPY(ParallelExporter)
};



#endif // PARALLELEXPORTER_H
