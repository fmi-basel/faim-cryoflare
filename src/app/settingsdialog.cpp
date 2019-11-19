//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2017-2018 by the CryoFLARE Authors
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

#include <QtDebug>
#include <QCheckBox>
#include "settings.h"
#include <QFileDialog>
#include <QtDebug>
#include <QStringList>
#include <QMenu>
#include <QComboBox>
#include <tasktreewidgetitem.h>
#include <variabletypes.h>
#include "settingsdialog.h"
#include "ui_settingsdialog.h"

namespace  {

template<typename SETTINGS>
void save_task(SETTINGS *settings, QTreeWidgetItem *item)
{
    TaskTreeWidgetItem *task_item=dynamic_cast<TaskTreeWidgetItem*>(item);
    if(task_item){
        QString name=task_item->name();
        settings->beginGroup(name);
        settings->setValue("script",task_item->script());
        settings->setValue("is_gpu",task_item->isGPU());
        settings->setValue("is_priority",task_item->isPriority());
        settings->setValue("group_with_parent",task_item->groupWithParent());
        QList<QVariant> variant_list;
        foreach(InputOutputVariable v,task_item->input_variables){
            variant_list<< v.toQVariant();
        }
        settings->setValue("input_variables",variant_list);
        variant_list.clear();
        foreach(InputOutputVariable v,task_item->output_variables){
            variant_list<< v.toQVariant();
        }
        settings->setValue("output_variables",variant_list);
    }
    for(int i=0;i<item->childCount();++i){
        save_task(settings,item->child(i));
    }
    if(task_item){
        settings->endGroup();
    }
}

template<typename SETTINGS>
void load_task(SETTINGS *settings, QTreeWidgetItem *parent)
{
    foreach(QString child_name,settings->childGroups()){
        TaskTreeWidgetItem *child=new TaskTreeWidgetItem(parent);
        settings->beginGroup(child_name);
        child->setName(child_name);
        child->setScript(settings->value("script").toString());
        child->setGpu(settings->value("is_gpu").toBool());
        child->setPriority(settings->value("is_priority").toBool());
        child->setGroupWithParent(settings->value("group_with_parent").toBool());
        QList<QVariant> variant_list=settings->value("input_variables").toList();
        foreach(QVariant v, variant_list){
            child->input_variables.append(InputOutputVariable(v));
        }
        variant_list=settings->value("output_variables").toList();
        foreach(QVariant v, variant_list){
            child->output_variables.append(InputOutputVariable(v));
        }
        load_task(settings,child);
        settings->endGroup();
    }
}

template<typename SETTINGS>
void save_to_settings(SETTINGS* settings, Ui::SettingsDialog* ui){
    QList<QCheckBox*> check_boxes= ui->default_columns->findChildren<QCheckBox*>();
    settings->beginGroup("DefaultColumns");
    Q_FOREACH(QCheckBox *cb, check_boxes){
        settings->setValue(cb->objectName(), cb->isChecked());
    }
    settings->endGroup();
    settings->setValue("num_cpu", ui->num_cpu->value());
    settings->setValue("num_gpu", ui->num_gpu->value());
    settings->setValue("gpu_ids", ui->gpu_ids->text());
    settings->setValue("timeout", ui->timeout->value());
    settings->setValue("histogram_bins", ui->histogram_bins->value());
    settings->setValue("import_image_pattern",ui->import_image_pattern->text());
    if(ui->import_epu->isChecked()){
        settings->setValue("import","EPU");
    }
    if(ui->import_flat_xml->isChecked()){
        settings->setValue("import","flat_EPU");
    }
    if(ui->import_json->isChecked()){
        settings->setValue("import","json");
    }
    settings->setValue("export_num_processes", ui->export_num_processes->value());
    settings->setValue("report_template",ui->report_template->path());
    settings->beginGroup("Tasks");
    settings->remove("");
    save_task(settings,ui->task_tree->invisibleRootItem());
    settings->endGroup();
}


template<typename SETTINGS>
void load_from_settings(SETTINGS* settings, Ui::SettingsDialog* ui){
    settings->beginGroup("DefaultColumns");
    Q_FOREACH( QCheckBox* cb, ui->default_columns->findChildren<QCheckBox*>()){
        cb->setChecked(settings->value(cb->objectName(),true).toBool());
    }
    settings->endGroup();
    ui->num_cpu->setValue(settings->value("num_cpu").toInt());
    ui->num_gpu->setValue(settings->value("num_gpu").toInt());
    ui->gpu_ids->setText(settings->value("gpu_ids").toString());
    ui->timeout->setValue(settings->value("timeout").toInt());
    ui->histogram_bins->setValue(settings->value("histogram_bins").toInt());
    ui->import_image_pattern->setText(settings->value("import_image_pattern").toString());
    QString import_mode=settings->value("import").toString();
    if(import_mode=="EPU"){
        ui->import_epu->setChecked(true);
    }else if(import_mode=="flat_EPU"){
        ui->import_flat_xml->setChecked(true);
    }else if(import_mode=="json"){
        ui->import_json->setChecked(true);
    }else{
        ui->import_epu->setChecked(true);
    }
    ui->report_template->setPath(settings->value("report_template").toString());
    settings->beginGroup("Tasks");
    ui->task_tree->clear();
    load_task(settings,ui->task_tree->invisibleRootItem());
    settings->endGroup();
    ui->task_tree->expandAll();
    ui->task_tree->resizeColumnToContents(0);

}

}

SettingsDialog::SettingsDialog(TaskConfiguration *task_config, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog),
    task_config_(task_config),
    task_tree_menu_(new QMenu(this)),
    task_tree_new_(new QAction("New",this)),
    task_tree_delete_(new QAction("Delete",this)),
    output_variable_new_(new QAction("New",this)),
    output_variable_delete_(new QAction("Delete",this)),
    input_variable_new_(new QAction("New",this)),
    input_variable_delete_(new QAction("Delete",this))
{
    ui->setupUi(this);
    ui->task_tree->setHeaderHidden(false);
    ui->task_tree->expandAll();
    ui->task_tree->resizeColumnToContents(0);
    ui->task_tree->resizeColumnToContents(2);
    connect(task_tree_new_, SIGNAL(triggered()), this, SLOT(newTask()));
    connect(task_tree_delete_, SIGNAL(triggered()), this, SLOT(deleteTask()));
    connect(ui->task_tree,SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),this,SLOT(updateVariables(QTreeWidgetItem*,QTreeWidgetItem*)));
    ui->task_tree->addAction(task_tree_new_);
    ui->task_tree->addAction(task_tree_delete_);
    ui->task_tree->setContextMenuPolicy(Qt::ActionsContextMenu);

    connect(output_variable_new_, SIGNAL(triggered()), this, SLOT(newOutputVariable()));
    connect(output_variable_delete_, SIGNAL(triggered()), this, SLOT(deleteOutputVariable()));
    ui->output_variable_table->addAction(output_variable_new_);
    ui->output_variable_table->addAction(output_variable_delete_);
    ui->output_variable_table->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->output_variable_table->setDisabled(true);
    connect(input_variable_new_, SIGNAL(triggered()), this, SLOT(newInputVariable()));
    connect(input_variable_delete_, SIGNAL(triggered()), this, SLOT(deleteInputVariable()));
    ui->input_variable_table->addAction(input_variable_new_);
    ui->input_variable_table->addAction(input_variable_delete_);
    ui->input_variable_table->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->input_variable_table->setDisabled(true);
    #if QT_VERSION >= 0x050000
        ui->task_tree->header()->setSectionResizeMode(1, QHeaderView::Stretch);
        ui->output_variable_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->input_variable_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    #else
        ui->task_tree->header()->setResizeMode(1, QHeaderView::Stretch);
        ui->output_variable_table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
        ui->input_variable_table->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    #endif
    QVBoxLayout *vbox = new QVBoxLayout;
    Q_FOREACH(InputOutputVariable column, task_config_->rootDefinition()->result_variables_){
        QCheckBox *cb=new QCheckBox(column.key,ui->default_columns);
        cb->setObjectName(column.label);
        vbox->addWidget(cb);
    }
    vbox->addStretch(1);
    ui->default_columns->setLayout(vbox);
    loadSettings();
}

void SettingsDialog::saveSettings()
{
    updateVariables(ui->task_tree->currentItem(),ui->task_tree->currentItem());
    Settings settings;
    save_to_settings(&settings,ui);
}

void SettingsDialog::loadSettings()
{
    Settings settings;
    load_from_settings(&settings,ui);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}


void SettingsDialog::newTask()
{
    if(ui->task_tree->currentItem()){
        QTreeWidgetItem *child=new TaskTreeWidgetItem(ui->task_tree->currentItem());
        ui->task_tree->currentItem()->addChild(child);
    }else{
        ui->task_tree->addTopLevelItem(new TaskTreeWidgetItem(ui->task_tree));
    }
    ui->task_tree->expandAll();
    ui->task_tree->resizeColumnToContents(0);
}

void SettingsDialog::deleteTask()
{
    if(ui->task_tree->currentItem()){
        delete ui->task_tree->currentItem();
    }
}

void SettingsDialog::newOutputVariable(const InputOutputVariable &variable)
{
    int row_count=ui->output_variable_table->rowCount();
    ui->output_variable_table->insertRow(row_count);
    ui->output_variable_table->setItem(row_count,0,new QTableWidgetItem());
    ui->output_variable_table->item(row_count,0)->setText(variable.key);
    ui->output_variable_table->setItem(row_count,1,new QTableWidgetItem());
    ui->output_variable_table->item(row_count,1)->setText(variable.label);
    QComboBox * combo_box=new QComboBox();
    combo_box->addItems(VariableTypeName);
    combo_box->setCurrentIndex(variable.type);
    ui->output_variable_table->setCellWidget(row_count,2,combo_box);
    ui->output_variable_table->setItem(row_count,3,new QTableWidgetItem());
    ui->output_variable_table->item(row_count,3)->setCheckState(variable.in_column ? Qt::Checked: Qt::Unchecked);
    ui->output_variable_table->item(row_count,3)->setFlags(ui->output_variable_table->item(row_count,3)->flags() & ~Qt::ItemIsEditable);
    combo_box=new QComboBox();
    combo_box->addItem("None");
    combo_box->addItem("Sum");
    combo_box->addItem("Average");
    combo_box->setCurrentIndex(variable.summary_type);
    ui->output_variable_table->setCellWidget(row_count,4,combo_box);
}

void SettingsDialog::deleteOutputVariable()
{
    ui->output_variable_table->removeRow(ui->output_variable_table->currentRow());
}

void SettingsDialog::newInputVariable(const InputOutputVariable &variable)
{
    int row_count=ui->input_variable_table->rowCount();
    ui->input_variable_table->insertRow(row_count);
    ui->input_variable_table->setItem(row_count,0,new QTableWidgetItem());
    ui->input_variable_table->item(row_count,0)->setText(variable.key);
    ui->input_variable_table->setItem(row_count,1,new QTableWidgetItem());
    ui->input_variable_table->item(row_count,1)->setText(variable.label);
    QComboBox * combo_box=new QComboBox();
    combo_box->addItems(VariableTypeName);
    combo_box->setCurrentIndex(variable.type);
    ui->input_variable_table->setCellWidget(row_count,2,combo_box);
    ui->input_variable_table->setItem(row_count,3,new QTableWidgetItem());
}

void SettingsDialog::deleteInputVariable()
{
    ui->input_variable_table->removeRow(ui->input_variable_table->currentRow());

}

void SettingsDialog::loadFromFile()
{
    QString path = QFileDialog::getOpenFileName(nullptr, "Open configuration file","","Ini files (*.ini);; All files (*)");
    if(! path.isEmpty()){
        QSettings* settings=new QSettings(path,QSettings::IniFormat);
        load_from_settings(settings,ui);
        delete settings;
    }
}

void SettingsDialog::saveToFile()
{
    updateVariables(ui->task_tree->currentItem(),ui->task_tree->currentItem());
    QString path = QFileDialog::getSaveFileName(nullptr, "Save configuration file","","Ini files (*.ini);; All files (*)");
    if(! path.isEmpty()){
        QSettings* settings=new QSettings(path,QSettings::IniFormat);
        save_to_settings(settings,ui);
        delete settings;
    }
}

void SettingsDialog::saveAsDefaults()
{
    saveSettings();
    Settings settings;
    settings.saveToQSettings(QStringList() << "avg_source_dir" << "stack_source_dir");
}

void SettingsDialog::resetToDefaults()
{
    Settings settings;
    settings.loadFromQSettings(QStringList() << "avg_source_dir" << "stack_source_dir");
    loadSettings();
}

void SettingsDialog::updateVariables(QTreeWidgetItem *new_item, QTreeWidgetItem *old_item)
{
    TaskTreeWidgetItem *old_tree_item=dynamic_cast<TaskTreeWidgetItem *>(old_item);
    TaskTreeWidgetItem *new_tree_item=dynamic_cast<TaskTreeWidgetItem *>(new_item);
    if(old_tree_item ){
        old_tree_item->output_variables.clear();
        for(int i=0;i<ui->output_variable_table->rowCount();++i){
            QString name=ui->output_variable_table->item(i,0)->text();
            QString variable=ui->output_variable_table->item(i,1)->text();
            QComboBox* combo_box=qobject_cast<QComboBox*>(ui->output_variable_table->cellWidget(i,2));
            VariableType type;
            if(combo_box){
                type=static_cast<VariableType>(combo_box->currentIndex());
            }
            bool is_column=ui->output_variable_table->item(i,3)->checkState()==Qt::Checked;
            combo_box=qobject_cast<QComboBox*>(ui->output_variable_table->cellWidget(i,4));
            InputOutputVariable::SummaryType summary_type;
            if(combo_box){
                summary_type=static_cast<InputOutputVariable::SummaryType>(combo_box->currentIndex());
            }else{
                summary_type=InputOutputVariable::NO_SUMMARY;
            }
            old_tree_item->output_variables.append(InputOutputVariable(name,variable,type,is_column,summary_type));
        }
        old_tree_item->input_variables.clear();
        for(int i=0;i<ui->input_variable_table->rowCount();++i){
            QString name=ui->input_variable_table->item(i,0)->text();
            QString variable=ui->input_variable_table->item(i,1)->text();
            QComboBox* combo_box=qobject_cast<QComboBox*>(ui->input_variable_table->cellWidget(i,2));
            VariableType type;
            if(combo_box){
                type=static_cast<VariableType>(combo_box->currentIndex());
            }
            old_tree_item->input_variables.append(InputOutputVariable(name,variable,type));
        }
    }
    ui->output_variable_table->setRowCount(0);
    ui->input_variable_table->setRowCount(0);
    if(new_tree_item){
        ui->input_variable_table->setDisabled(false);
        ui->output_variable_table->setDisabled(false);
        foreach(InputOutputVariable v,new_tree_item->input_variables){
            newInputVariable(v);
        }
        foreach(InputOutputVariable v,new_tree_item->output_variables){
            newOutputVariable(v);
        }
    }else{
        ui->input_variable_table->setDisabled(true);
        ui->output_variable_table->setDisabled(true);

    }
}

void SettingsDialog::designReport()
{
    LimeReport::ReportEngine report_engine;
    Settings settings;
    if(QFileInfo::exists(settings.value("report_template").toString())){
      report_engine.loadFromFile(settings.value("report_template").toString());
    }
    report_engine.designReport();
    ui->report_template->setPath(report_engine.reportFileName());
}

