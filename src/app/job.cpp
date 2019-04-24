#include "job.h"

Job::Job()
{

}

void Job::start(const QVector<int> &cpu_ids_, const QVector<int> &gpu_ids_)
{

}

bool Job::usesGPU() const
{
    return ngpu>0;
}
