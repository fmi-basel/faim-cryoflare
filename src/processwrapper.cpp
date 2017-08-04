#include <QtDebug>
#include <QProcessEnvironment>
#include <QCoreApplication>
#include <QTextStream>
#include "settings.h"
#include "processwrapper.h"

ProcessWrapper::ProcessWrapper(QObject *parent, int gpu_id) :
    QObject(parent),
    process_(new QProcess),
    task_(),
    gpu_id_(gpu_id),
    running_(false)
{
    connect(process_,SIGNAL(finished(int)),this,SLOT(onFinished(int)));
    QProcessEnvironment env=QProcessEnvironment::systemEnvironment();
    env.insert("STACK_GUI_SCRIPTS",QCoreApplication::applicationDirPath()+"/scripts");
    process_->setProcessEnvironment(env);
}

bool ProcessWrapper::running() const
{
    return running_;
}

void ProcessWrapper::start(const TaskPtr &task)
{
    running_=true;
    task_=task;
    process_->start(task_->script);
    foreach(QString key,task_->data->keys()){
        QString val=task_->data->value(key);
        process_->write(QString("%1=%2\n").arg(key,val).toLatin1());
    }
    if(-1!=gpu_id_){
        process_->write(QString("gpu_id=%1\n").arg(gpu_id_).toLatin1());
    }
    Settings settings;
    settings.beginGroup("ScriptInput");
    settings.beginGroup(task->name);
    foreach(QString name,settings.allKeys()){
        QString value=settings.value(name).toString();
        process_->write(QString("%1=%2\n").arg(name,value).toLatin1());
    }
    settings.endGroup();
    settings.endGroup();
    process_->closeWriteChannel();

}

void ProcessWrapper::onFinished(int exitcode)
{
    QTextStream output_stream(process_->readAllStandardOutput(),QIODevice::ReadOnly);
    QString result_token("RESULT_EXPORT:");
    QString result_file_token("FILE_EXPORT:");
    QString shared_result_file_token("SHARED_FILE_EXPORT:");
    QString output;
    QString line;
    do {
        line = output_stream.readLine();
        if(line.startsWith(result_token)){
            line.remove(0,result_token.size());
            QStringList splitted=line.split("=");
            task_->data->insert(splitted[0].trimmed(),splitted[1].trimmed());
        }else if(line.startsWith(result_file_token)){
            line.remove(0,result_file_token.size());
            task_->output_files.insert(line.trimmed());
        }else if(line.startsWith(shared_result_file_token)){
            line.remove(0,shared_result_file_token.size());
            task_->shared_output_files.insert(line.trimmed());
        }else{
            output.append(line+"\n");
        }
    } while (!line.isNull());
    task_->output=output;
    task_->error=process_->readAllStandardError();
    task_->state=exitcode;
    TaskPtr task=task_;
    task_.clear();
    running_=false;
    emit finished(task,-1!=gpu_id_);
}
