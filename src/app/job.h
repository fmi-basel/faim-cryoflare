#ifndef JOB_H
#define JOB_H

#include <QObject>

class Job
{
public:
    Job();
    void start(const QVector<int>& cpu_ids_, const QVector<int>& gpu_ids_);
    int pid;
    int ncpu;
    int ngpu;
    int data_id;
    int template_id;
    bool usesGPU() const;
};

#endif // JOB_H
