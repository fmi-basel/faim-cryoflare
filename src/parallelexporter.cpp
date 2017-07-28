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
            process.waitForFinished(-1);
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
    num_tasks_(0),
    num_tasks_done_(0),
    dialog_(NULL),
    mutex_(),
    num_threads_(),
    post_script_(),
    post_arguments_()

{

}

void ParallelExporter::exportImages(const QString &source, const QString &destination, QQueue<QSet<QString> > &image_list, int num_processes, const QString &mode, const QString &custom_script, const QString &pre_script, const QString &post_script,const QStringList& image_names)
{
    if(!queue_.empty()){
        return;
    }
    post_script_=post_script;
    queue_=image_list;
    num_tasks_=image_list.size()+2;

    if (qobject_cast<QApplication*>(QCoreApplication::instance())) {
        //GUI
        dialog_=new QProgressDialog("Exporting images...", "Abort Export", 0, num_tasks_,QApplication::topLevelWidgets()[0]);
        dialog_->setWindowModality(Qt::ApplicationModal);
        dialog_->setMinimumDuration(0);
        connect(dialog_, SIGNAL(canceled()), this, SLOT(cancel()));
    }else{
        std::cerr << "[" << std::string(78,'-') << "]\r";
    }

    QStringList pre_post_arguments;
    pre_post_arguments << source << destination;
    pre_post_arguments+=image_names;
    post_arguments_=pre_post_arguments;
    if(pre_script.size()){
        QProcess process;
        process.start(pre_script,pre_post_arguments);
        process.waitForFinished(-1);
    }
    updateProgress();
    num_threads_=0;
    for(int i=0;i<num_processes && i< image_list.size();++i){
        ++num_threads_;
        QThread* thread = new QThread(this);
        Worker* worker = new Worker(source,destination,&queue_,&mutex_,mode,custom_script);
        worker->moveToThread(thread);
        connect(worker, SIGNAL (nextImage()), this, SLOT (updateProgress()));
        connect(thread, SIGNAL (started()), worker, SLOT (process()));
        connect(worker, SIGNAL (finished()), this, SLOT (runPost()));
        connect(worker, SIGNAL (finished()), thread, SLOT (quit()));
        connect(worker, SIGNAL (finished()), worker, SLOT (deleteLater()));
        connect(thread, SIGNAL (finished()), thread, SLOT (deleteLater()));
        thread->start();
    }
}



void ParallelExporter::updateProgress()
{
    ++num_tasks_done_;
    qDebug() << "num tasks done: " << num_tasks_done_ <<"/" << num_tasks_;
    if(dialog_){
        dialog_->setValue(num_tasks_done_);
    }else{
        if(num_tasks_done_==num_tasks_){
            std::cerr << "[" << std::string(78,'#') << "]\n";
        }else{
            int progressed=num_tasks_done_*78/num_tasks_;
            std::cerr << "[" << std::string(progressed,'#') << std::string(78-progressed,'-') << "]\r";
        }
    }

}

void ParallelExporter::cancel()
{
    QMutexLocker locker(&mutex_);
    queue_.clear();
}

void ParallelExporter::runPost()
{
    qDebug() << "checking run post" ;
    --num_threads_;
    if(!num_threads_){
        qDebug() << "run post" ;
        if(post_script_.size()){
            QProcess process;
            process.start(post_script_,post_arguments_);
            process.waitForFinished(-1);
        }
        updateProgress();
    }
}
