#include "task.h"



Task::Task(const QString &name_, const QString &script_, DataPtr data_, bool gpu_):
    name(name_),
    script(script_),
    data(data_),
    gpu(gpu_),
    output(),
    error(),
    state(-1),
    children(),
    display_keys(),
    display_details()
{
    display_details << QStringList() << QStringList() << QStringList();
}

void Task::setData(const DataPtr &data_)
{
    data=data_;
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
    display_details[0].append(key);
    display_details[1].append(label);
    display_details[2].append(type);
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
