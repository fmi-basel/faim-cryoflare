#include <iostream>
#include <QDir>
#include <QDebug>
#include <QApplication>
#include <QProgressDialog>
#include <QProcess>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include "parallelexporter.h"

Worker::Worker(const QString &source, const QString &destination, QQueue<QSet<QString> > *queue, QMutex *mutex, const QString &mode, const QString &script,QObject *parent):
    QObject(parent),
    source_(source),
    destination_(destination),
    queue_(queue),
    mutex_(mutex),
    mode_(mode),
    script_(script)
{
}

Worker::~Worker(){}

void Worker::process() {
    while(true){
        QSet<QString> file_list;
        {
            QMutexLocker locker(mutex_);
            if(queue_->empty()){
                emit finished();
                return;
            }
            file_list=queue_->dequeue();
        }
        if(mode_=="custom"){
            QProcess process;
            QStringList arguments;
            arguments << source_ << destination_;
            arguments+=file_list.values();
            process.start(script_,arguments);
            process.waitForFinished();
        }else if(mode_=="move"){
            foreach(QString relative_file,file_list){
                QFile(QDir(source_).absoluteFilePath(relative_file)).rename(QDir(destination_).absoluteFilePath(relative_file));
            }
        }else{
            foreach(QString relative_file,file_list){
                QFile(QDir(source_).absoluteFilePath(relative_file)).copy(QDir(destination_).absoluteFilePath(relative_file));
            }
        }
        emit nextImage();
    }
}


ParallelExporter::ParallelExporter(QObject *parent):
    QObject(parent),
    queue_(),
    num_images_(0),
    num_images_done_(0),
    dialog_(NULL),
    mutex_()
{

}

void ParallelExporter::exportImages(const QString &source, const QString &destination, QQueue<QSet<QString> > &image_list, int num_processes, const QString &mode, const QString &script)
{
    if(!queue_.empty()){
        return;
    }
    queue_=image_list;
    num_images_=image_list.size();
    for(int i=0;i<num_processes && i< num_images_;++i){
        QThread* thread = new QThread(this);
        Worker* worker = new Worker(source,destination,&queue_,&mutex_,mode,script);
        worker->moveToThread(thread);
        connect(worker, SIGNAL (nextImage()), this, SLOT (updateProgress()));
        connect(thread, SIGNAL (started()), worker, SLOT (process()));
        connect(worker, SIGNAL (finished()), thread, SLOT (quit()));
        connect(worker, SIGNAL (finished()), worker, SLOT (deleteLater()));
        connect(thread, SIGNAL (finished()), thread, SLOT (deleteLater()));
        thread->start();
    }
    if (qobject_cast<QApplication*>(QCoreApplication::instance())) {
        //GUI
        dialog_=new QProgressDialog("Exporting images...", "Abort Export", 0, num_images_);
        dialog_->setWindowModality(Qt::ApplicationModal);
        connect(dialog_, SIGNAL(canceled()), this, SLOT(cancel()));
    }else{
        std::cerr << "[" << std::string(78,'-') << "]\r";
    }
}


void ParallelExporter::updateProgress()
{
    ++num_images_done_;
    if(dialog_){
        dialog_->setValue(num_images_done_);
    }else{
        if(num_images_done_==num_images_){
            std::cerr << "[" << std::string(78,'#') << "]\n";
        }else{
            int progressed=num_images_done_*78/num_images_;
            std::cerr << "[" << std::string(progressed,'#') << std::string(78-progressed,'-') << "]\r";
        }
    }

}

void ParallelExporter::cancel()
{
    QMutexLocker locker(&mutex_);
    queue_.clear();
}
