#ifndef PROCESSQUEUE_H
#define PROCESSQUEUE_H

#include <QObject>
#include "priorityqueue.h"
#include "metadatastore.h"

//fw decl
class ProcessWrapper;

class ProcessQueue : public QObject
{
    Q_OBJECT
public:
    explicit ProcessQueue(MetaDataStore* meta_data_store,QObject *parent = nullptr);
    virtual ~ProcessQueue();
    void enqueue(const TaskPtr& task);
    void start();
    void stop();
    void stopProcess(const QString& id);
    int size();
    void createProcesses(int num_processes=0, int timeout=300,QStringList gpus=QStringList());
    QList<ProcessWrapper*> processes_;
    QList<ProcessWrapper*> idle_processes_;
    PriorityQueue pqueue_;
    bool running_;
protected:
    MetaDataStore * meta_data_store_;
signals:
    void taskSucceded(const QString &id, const TaskDefinitionPtr &taskdef);
    void taskFailed(const QString &id, const TaskDefinitionPtr &taskdef, int exitcode);
public slots:
    void onProcessFinished(ProcessWrapper* process, const TaskPtr &task, int exitcode);
};

#endif // PROCESSQUEUE_H
