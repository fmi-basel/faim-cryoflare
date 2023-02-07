//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2020 by the CryoFLARE Authors
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
#ifndef TASKTREEWIDGETITEM_H
#define TASKTREEWIDGETITEM_H

#include <inputoutputvariable.h>
#include <QTreeWidgetItem>

//fw decl
class PathEdit;
class QCheckBox;

class TaskTreeWidgetItem : public QTreeWidgetItem
{
public:
    explicit TaskTreeWidgetItem(QTreeWidget *parent);
    explicit TaskTreeWidgetItem(QTreeWidgetItem *parent);
    QString name() const;
    QString script() const;
    bool isGPU() const;
    bool isPriority() const;
    bool groupWithParent() const;
    void setName(const QString &name);
    void setScript(const QString& script);
    void setGpu(bool gpu);
    void setPriority(bool p);
    void setGroupWithParent(bool group);
    QList<InputOutputVariable> input_variables;
    QList<InputOutputVariable> output_variables;

signals:

public slots:
    void onPathBrowse();
private:
    void init_();
    PathEdit *path_widget_;
    QCheckBox *gpu_check_box_;
    QCheckBox *priority_check_box_;
    QCheckBox *group_with_parent_;
};

#endif // TASKTREEWIDGETITEM_H
