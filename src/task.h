#ifndef TASK_H
#define TASK_H

#include <dataptr.h>
#include <QPair>
#include <QStringList>
#include <QString>
#include <QSet>

//fw decl
class Task;

typedef QSharedPointer<Task> TaskPtr;

class DisplayDetail{
public:
    DisplayDetail(const QString& key_, const QString &label_,const QString& type_):
        key(key_),
        label(label_),
        type(type_)
    {}
    QString key;
    QString label;
    QString type;
};

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
    QSet<QString> raw_files;
    QSet<QString> output_files;
    QSet<QString> shared_output_files;
    int state;
    QList<TaskPtr > children;
    QPair<QStringList,QStringList> display_keys;
    QList<DisplayDetail> display_details;
};

#endif // TASK_H
