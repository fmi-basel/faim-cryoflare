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

void TaskTreeWidgetItem::setGroupWithParent(bool group)
{
    group_with_parent_->setChecked(group);
}


void TaskTreeWidgetItem::init_()
{
    treeWidget()->setItemWidget(this,1,path_widget_);
    treeWidget()->setItemWidget(this,2,gpu_check_box_);
    treeWidget()->setItemWidget(this,3,group_with_parent_);
    setFlags(flags() | Qt::ItemIsEditable);
}
