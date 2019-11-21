#include "processqueue.h"
#include "processwrapper.h"

ProcessQueue::ProcessQueue(MetaDataStore *meta_data_store, QObject *parent) :
    QObject(parent),
    processes_(),
    idle_processes_(),
    pqueue_(),
    running_(false),
    meta_data_store_(meta_data_store)
{
}

ProcessQueue::~ProcessQueue()
{
    foreach (ProcessWrapper* process, processes_) {
        process->terminate();
    }
}

void ProcessQueue::enqueue(const TaskPtr &task)
{
    if(!running_ || idle_processes_.empty()){
        pqueue_.enqueue(task,task->definition->priority);
    }else{
        idle_processes_.takeFirst()->start(task);
    }
}

void ProcessQueue::start()
{
    running_=true;
    while(!pqueue_.empty() && !idle_processes_.empty()){
        idle_processes_.takeFirst()->start(pqueue_.dequeue());
    }
}

void ProcessQueue::stop()
{
    running_=false;
    foreach(ProcessWrapper* process, processes_){
        // terminate and re-enqueue task
        if(process->task()!=nullptr){
            pqueue_.enqueue(process->task());
        }
        if(process->running()){
            process->terminate();
        }
        if(idle_processes_.indexOf(process)==-1){
            idle_processes_.append(process);
        }
    }
}

void ProcessQueue::stopProcess(const QString &id)
{
    foreach(ProcessWrapper* wrapper,processes_){
        if(!wrapper->running()){
            continue;
        }
        if(wrapper->task()->id==id){
            wrapper->terminate();
        }
    }
}

int ProcessQueue::size()
{
    return pqueue_.size()+processes_.size()-idle_processes_.size();
}

void ProcessQueue::createProcesses(int num_processes, int timeout, QStringList gpus)
{
    foreach (ProcessWrapper* process, processes_) {
        process->terminate();
        process->deleteLater();
    }
    processes_.clear();
    if(gpus.empty()){
        gpus << "-1";
    }
    for(int i=0;i<num_processes;++i) {
        int gpu_id=gpus.at(i%gpus.size()).toInt();
        ProcessWrapper* wrapper=new ProcessWrapper(this, meta_data_store_, timeout,gpu_id);
        connect(wrapper,&ProcessWrapper::finished,this,&ProcessQueue::onProcessFinished);
        processes_.append(wrapper);
        idle_processes_.append(wrapper);
    }
}

void ProcessQueue::onProcessFinished(ProcessWrapper *process, const TaskPtr &task,int exitcode)
{
    if(!pqueue_.empty()){
        process->start(pqueue_.dequeue());
    }else{
        idle_processes_.append(process);
    }
    if(exitcode==0){
        emit taskSucceded(task->id,task->definition);
    }else{
        emit taskFailed(task->id,task->definition,exitcode);
    }
}
