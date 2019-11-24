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

#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QStringList>
#include <QCheckBox>
#include <pathedit.h>

#include "tasktreewidgetitem.h"

TaskTreeWidgetItem::TaskTreeWidgetItem(QTreeWidget *parent) :
    QTreeWidgetItem(parent, QStringList("Task")),
    input_variables(),
    output_variables(),
    path_widget_(new PathEdit(PathEdit::OpenFileName,"Open script file","","Scripts (*.sh *.csh *.py);;All files (*)")),
    gpu_check_box_(new QCheckBox()),
    priority_check_box_(new QCheckBox()),
    group_with_parent_(new QCheckBox())
{
    init_();
}

TaskTreeWidgetItem::TaskTreeWidgetItem(QTreeWidgetItem *parent):
    QTreeWidgetItem(parent, QStringList("Task")),
    input_variables(),
    output_variables(),
    path_widget_(new PathEdit(PathEdit::OpenFileName,"Open script file","","Scripts (*.sh *.csh *.py);;All files (*)")),
    gpu_check_box_(new QCheckBox()),
    priority_check_box_(new QCheckBox()),
    group_with_parent_(new QCheckBox())
{
    init_();
}

QString TaskTreeWidgetItem::name() const
{
    return data(0,Qt::DisplayRole).toString();
}

QString TaskTreeWidgetItem::script() const
{
    return path_widget_->path();
}

bool TaskTreeWidgetItem::isGPU() const
{
    return gpu_check_box_->checkState()==Qt::Checked;
}

bool TaskTreeWidgetItem::isPriority() const
{
    return priority_check_box_->checkState()==Qt::Checked;
}

bool TaskTreeWidgetItem::groupWithParent() const
{
    return group_with_parent_->checkState()==Qt::Checked;
}

void TaskTreeWidgetItem::setName(const QString &name)
{
    setData(0,Qt::DisplayRole,name);
}

void TaskTreeWidgetItem::setScript(const QString &script)
{
    path_widget_->setPath(script);
}

void TaskTreeWidgetItem::setGpu(bool gpu)
{
    gpu_check_box_->setChecked(gpu);
}

void TaskTreeWidgetItem::setPriority(bool p)
{
    priority_check_box_->setChecked(p);
}

void TaskTreeWidgetItem::setGroupWithParent(bool group)
{
    group_with_parent_->setChecked(group);
}


void TaskTreeWidgetItem::init_()
{
    treeWidget()->setItemWidget(this,1,path_widget_);
    treeWidget()->setItemWidget(this,2,gpu_check_box_);
    treeWidget()->setItemWidget(this,3,priority_check_box_);
    treeWidget()->setItemWidget(this,4,group_with_parent_);
    setFlags(flags() | Qt::ItemIsEditable);
}
