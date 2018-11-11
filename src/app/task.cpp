//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFlare
//
// Copyright (C) 2017-2018 by the CryoFlare Authors
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
// along with CryoFlare.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------------

#include "task.h"



Task::Task(const QString &name_, const QString &script_, DataPtr data_, bool gpu_):
    name(name_),
    script(script_),
    data(data_),
    gpu(gpu_),
    output(),
    error(),
    raw_files(),
    output_files(),
    shared_output_files(),
    state(-1),
    children(),
    display_keys(),
    display_details()
{
}

void Task::setData(const DataPtr &data_)
{
    data=data_;
    data->insert("tasks_unfinished",1+data->value("tasks_unfinished").toInt(0));
    foreach(TaskPtr child,children){
        child->setData(data_);
    }
}

void Task::addColumn(const QString &key, const QString &value)
{
    display_keys.first.append(key);
    display_keys.second.append(value);
}

void Task::addDetail(const QString &key,const QString &label, const QString &type)
{
    display_details.append(DisplayDetail(key,label,type));
}

QPair<QStringList,QStringList> Task::getDisplayKeys() const
{
   QPair<QStringList,QStringList> result=display_keys;
   for(int i=0;i<children.size();++i){
       QPair<QStringList,QStringList> child_result=children[i]->getDisplayKeys();
       for(int j=0;j<child_result.first.size();++j){
           if(!result.first.contains(child_result.first[j])){
               result.first.append(child_result.first[j]);
               result.second.append(child_result.second[j]);
           }
       }
   }
   return result;
}

TaskPtr Task::clone()
{
    TaskPtr ptr(new Task(name,script,data,gpu));
    ptr->output=output;
    ptr->error=error;
    ptr->state=state;
    foreach(TaskPtr child, children){
        ptr->children.append(child->clone());
    }
    return ptr;
}
