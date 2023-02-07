//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2017-2020 by the CryoFLARE Authors
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

#include <QTimer>
#include "task.h"
#include "settings.h"


TaskDefinition* load_child_task_(TaskDefinition* parent, const QString& name, Settings *settings){
    static QList<QColor> colors= QList<QColor>() << QColor(155,140,140)<< QColor(140,155,140)<< QColor(140,155,155)<< QColor(140,140,155)<< QColor(155,155,140)<< QColor(155,140,155);
    static int color_idx=0;
    settings->beginGroup(name);
    TaskDefinition* definition=new TaskDefinition(parent,
                                                  name,
                                                  settings->value("script","").toString(),
                                                  settings->value("is_gpu",false).toBool(),
                                                  settings->value("is_priority",false).toBool(),
                                                  settings->value("group_with_parent",false).toBool());
    if(definition->group_with_parent){
        definition->color=parent->color;
    }else{
        definition->color=colors[color_idx];
        ++color_idx;
        color_idx%=colors.size();
    }
    QList<QVariant> variant_list=settings->value("output_variables").toList();
    foreach(QVariant v,variant_list){
        definition->result_variables_.append(InputOutputVariable(v));
    }
    variant_list=settings->value("input_variables").toList();
    foreach(QVariant v,variant_list){
        definition->input_variables_.append(InputOutputVariable(v));
    }
    foreach(QString child_name,settings->childGroups()){
        definition->children.append(TaskDefinitionPtr(load_child_task_(definition,child_name,settings)));
    }
    settings->endGroup();
    return definition;
}

TaskDefinitionPtr TaskDefinition::loadTaskDefinitions(Settings *settings)
{
    TaskDefinitionPtr root_definition(new TaskDefinition(nullptr,"root","",false,false,false));
    root_definition->color=QColor(136, 138, 133);
    static QList<InputOutputVariable> default_results=  QList<InputOutputVariable>()
                                                        << InputOutputVariable("Name","short_name",String)
                                                        << InputOutputVariable("Timestamp","timestamp",String)
                                                        << InputOutputVariable("Nom. Defocus","defocus",Float)
                                                        << InputOutputVariable("Exp. time","exposure_time",Float)
                                                        << InputOutputVariable("Measured Dose","dose",Float)
                                                        << InputOutputVariable("Pixel size","apix_x",Float)
                                                        << InputOutputVariable("# Frames","num_frames",Int)
                                                        << InputOutputVariable("PP","phase_plate_num",Float)
                                                        << InputOutputVariable("PP position","phase_plate_pos",Float)
                                                        << InputOutputVariable("PP count","phase_plate_count",Float)
                                                        << InputOutputVariable("Grid square X","square_x",Float)
                                                        << InputOutputVariable("Grid square Y","square_y",Float)
                                                        << InputOutputVariable("Grid square Z","square_z",Float)
                                                        << InputOutputVariable("Grid square id","square_id",Float)
                                                        << InputOutputVariable("X","x",Float)
                                                        << InputOutputVariable("Y","y",Float)
                                                        << InputOutputVariable("Z","z",Float)
                                                        << InputOutputVariable("Hole Position X","hole_x",Float)
                                                        << InputOutputVariable("Hole Position Y","hole_y",Float);
    settings->beginGroup("DefaultColumns");
    foreach(InputOutputVariable v, default_results){
        root_definition->result_variables_.append(v);
        root_definition->result_variables_.last().in_column=settings->value(v.label,true).toBool();
    }
    settings->endGroup();
    settings->beginGroup("Tasks");
    foreach(QString child_name,settings->childGroups()){
        root_definition->children.append(TaskDefinitionPtr(load_child_task_(root_definition.data(), child_name,settings)));
    }
    settings->endGroup();
    return root_definition;
}

TaskDefinition::TaskDefinition(TaskDefinition* parent_, const QString &name_, const QString &script_, bool gpu_, bool priority_, bool group_with_parent_):
    parent(parent_),
    name(name_),
    script(script_),
    gpu(gpu_),
    priority(priority_),
    group_with_parent(group_with_parent_),
    color(),
    input_variables_(),
    result_variables_(),
    children()
{

}

QString TaskDefinition::taskString() const
{
    return QString("_CryoFLARE_TASK_%1").arg(name);
}




TaskConfiguration::TaskConfiguration(QObject *parent):
    QObject (parent),
    root_definition_(new TaskDefinition(nullptr,"root","",false,false,false))
{
    QTimer::singleShot(0, this, &TaskConfiguration::updateConfiguration);
}

TaskDefinitionPtr TaskConfiguration::rootDefinition()
{
    return root_definition_;
}

QList<InputOutputVariable> TaskConfiguration::resultLabels()
{
    QList<InputOutputVariable> result;
    QList<TaskDefinitionPtr> def_list;
    def_list.append(root_definition_);
    //Warning: ordering here has to match code in ImageTableModel
    while(!def_list.empty()){
        TaskDefinitionPtr ptr=def_list.takeFirst();
        result.append(ptr->result_variables_);
        QList<TaskDefinitionPtr> children=ptr->children;
        while (!children.empty()) {
            def_list.push_front(children.takeLast());
        }
    }
    return result;
}

void TaskConfiguration::updateConfiguration()
{
    Settings *settings=new Settings;
    TaskDefinitionPtr nptr=TaskDefinition::loadTaskDefinitions(settings);
    root_definition_.swap(nptr);
    emit configurationChanged();
}

Task::Task(const TaskDefinitionPtr &definition_, const QString &id_):
    definition(definition_),
    id(id_)
{

}
