//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2019 by the CryoFLARE Authors
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3.0 of the License.
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License
// along with CryoFLARE.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------------
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
