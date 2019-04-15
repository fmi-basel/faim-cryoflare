#ifndef PRIORITYQUEUE_H
#define PRIORITYQUEUE_H

#include <QMultiMap>
#include "task.h"

class PriorityQueue
{
public:
    PriorityQueue();
    void enqueue(const TaskPtr& task, bool priority=false);
    TaskPtr dequeue();
    void clear();
    TaskPtr head() const;
    bool empty() const;
    int size() const;
protected:
    QMultiMap<uint,TaskPtr> mmap_;
};

#endif // PRIORITYQUEUE_H
