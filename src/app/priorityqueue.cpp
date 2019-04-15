#include <climits>
#include "priorityqueue.h"

PriorityQueue::PriorityQueue():
    mmap_()
{

}

void PriorityQueue::enqueue(const TaskPtr &task, bool priority)
{
    if(priority){
        mmap_.insert(0,task);
    }else{
        QMultiMap<uint,TaskPtr>::iterator it=mmap_.upperBound(0);
        if(it==mmap_.end()){
            mmap_.insert(UINT_MAX,task);
        }else{
            mmap_.insert(it.key()>0?it.key()-1:0,task);
        }
    }
}

TaskPtr PriorityQueue::dequeue()
{
    TaskPtr task=mmap_.begin().value();
    mmap_.erase(mmap_.begin());
    return task;
}

void PriorityQueue::clear()
{
    mmap_.clear();
}

TaskPtr PriorityQueue::head() const
{
    return mmap_.begin().value();  ;
}

bool PriorityQueue::empty() const
{
    return mmap_.empty();
}

int PriorityQueue::size() const
{
    return mmap_.size();
}
