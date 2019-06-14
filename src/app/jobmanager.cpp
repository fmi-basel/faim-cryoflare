#include "jobmanager.h"
#include <QDebug>
JobManager::JobManager(QObject *parent) : QObject(parent)
{

}

void JobManager::submit(Job &j)
{
    int n_cpu=numCoresFree_(cpu_cores_);
    int n_gpu=numCoresFree_(gpu_cores_);
    if(j.ncpu<=n_cpu && j.ngpu<=n_gpu){
        start_(j);
    }else{
        if(j.usesGPU()){
            gpu_queue_.append(j);
        }else{
            cpu_queue_.append(j);
        }
    }
}

int JobManager::numCoresFree_(QList<Core>& core_list) const
{
    int n=0;
    foreach(Core c, core_list){
        if(c.pid==-1){
            ++n;
        }
    }
    return n;
}

void JobManager::jobFinished(const Job &/*j*/)
{

}

void JobManager::start_( Job &j)
{
    QVector<int> cpu_ids=acquireCores_(cpu_cores_,j.ncpu);
    if(cpu_ids.size()!=j.ncpu){
        qDebug() << "failed to reserve CPUs";
        return;
    }
    QVector<int> gpu_ids=acquireCores_(gpu_cores_,j.ngpu);
    if(gpu_ids.size()!=j.ngpu){
        qDebug() << "failed to reserve GPUs";
        return;
    }
    j.start(cpu_ids,gpu_ids);
}

QVector<int> JobManager::acquireCores_(QList<Core>& core_list,int num)
{
    QVector<int> result;
    for(int i=0;i<core_list.size();++i){
        if(core_list[i].pid==-1){
            core_list[i].pid=0;
            result.append(i);
            if(result.size()==num){
                break;
            }
        }
    }
    if(result.size()<num){
        releaseCores_(core_list,result);
        return QVector<int>();
    }
    return result;
}

void JobManager::releaseCores_(QList<Core> &core_list, const QVector<int> &ids)
{
    foreach(int id,ids){
        core_list[id].pid=-1;
    }
}

