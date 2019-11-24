//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2017-2019 by the CryoFLARE Authors
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

#ifndef TASK_H
#define TASK_H

#include <dataptr.h>
#include "inputoutputvariable.h"
#include <QPair>
#include <QStringList>
#include <QString>
#include <QMap>
#include <QColor>

//fw decl
class Task;
class Settings;
class TaskDefinition;
class TaskConfiguration;


typedef QSharedPointer<Task> TaskPtr;
typedef QSharedPointer<TaskDefinition> TaskDefinitionPtr;
typedef QSharedPointer<TaskConfiguration> TaskConfigurationPtr;


class TaskDefinition
{
public:
    static TaskDefinitionPtr loadTaskDefinitions(Settings *settings);
    explicit TaskDefinition(TaskDefinition* parent_, const QString& name_, const QString& script_, bool gpu_, bool priority_, bool group_with_parent_);
    QString taskString() const;
    TaskDefinition* parent;
    QString name;
    QString script;
    bool gpu;
    bool priority;
    bool group_with_parent;
    QColor color;
    QList<InputOutputVariable> input_variables_;
    QList<InputOutputVariable> result_variables_;
    QList<TaskDefinitionPtr> children;
};

class TaskConfiguration: public QObject
{
    Q_OBJECT
public:
    TaskConfiguration(QObject * parent=nullptr);
    TaskDefinitionPtr rootDefinition();
    QList<InputOutputVariable> resultLabels();
public slots:
    void updateConfiguration();
signals:
    void configurationChanged();
protected:
    TaskDefinitionPtr root_definition_;
};

class Task
{
public:
    explicit Task(const TaskDefinitionPtr& definition_, const QString& id_);
    TaskDefinitionPtr definition;
    QString id;


};

#endif // TASK_H
