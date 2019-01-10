#ifndef JOBMANAGER_H
#define JOBMANAGER_H

#include <QList>
#include <QStack>
#include <QObject>
#include "job.h"

struct Core{
    int pid;
    int start_time;
};


class JobManager : public QObject
{
    Q_OBJECT
public:
    explicit JobManager(QObject *parent = nullptr);
    void submit( Job& j);

signals:

public slots:
    void jobFinished(const Job& j);
protected:
    void start_(Job &j);
    int numCoresFree_(QList<Core>& core_list) const;
    QVector<int> acquireCores_(QList<Core>& core_list,int num);
    void releaseCores_(QList<Core>& core_list,const QVector<int>& ids);
protected:
    QList<Core> cpu_cores_;
    QList<Core> gpu_cores_;
    QList<Job>  running_jobs_;
    QStack<Job> gpu_queue_;
    QStack<Job> cpu_queue_;

};

#endif // JOBMANAGER_H
