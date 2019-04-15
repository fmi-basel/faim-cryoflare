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

#ifndef TASK_H
#define TASK_H

#include <dataptr.h>
#include <QPair>
#include <QStringList>
#include <QString>
#include <QMap>

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
    explicit Task(const QString& name_, const QString& script_, DataPtr data_, bool gpu_=false, bool priority_=false);
    void setData(const DataPtr &data_,bool force_reprocess=false);
    void addColumn(const QString& key, const QString& value);
    void addDetail(const QString& key, const QString &label,const QString& type);
    QString taskString() const;
    QPair<QStringList,QStringList> getDisplayKeys() const;
    TaskPtr clone();
    QString name;
    QString script;
    DataPtr data;
    bool gpu;
    bool priority;
    QString output;
    QString error;
    QMap<QString,QString> raw_files;
    QMap<QString,QString> output_files;
    QMap<QString,QString> shared_output_files;
    int state;
    QList<TaskPtr > children;
    QPair<QStringList,QStringList> display_keys;
    QList<DisplayDetail> display_details;
};

#endif // TASK_H
