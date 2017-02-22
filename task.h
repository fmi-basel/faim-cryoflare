#ifndef TASK_H
#define TASK_H

#include <dataptr.h>
#include <QPair>
#include <QStringList>
#include <QString>

//fw decl
class Task;

typedef QSharedPointer<Task> TaskPtr;


class Task
{
public:
    explicit Task(const QString& name_, const QString& script_,DataPtr data_, bool gpu_=0);
    void setData(const DataPtr &data_);
    void addColumn(const QString& key, const QString& value);
    void addDetail(const QString& key, const QString &label,const QString& type);
    QPair<QStringList,QStringList> getDisplayKeys() const;
    TaskPtr clone();
    QString name;
    QString script;
    DataPtr data;
    bool gpu;
    QString output;
    QString error;
    int state;
    QList<TaskPtr > children;
    QPair<QStringList,QStringList> display_keys;
    QList<QStringList> display_details;
};

#endif // TASK_H
