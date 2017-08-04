#include <QtDebug>
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
    settings->setValue("num_cpu", ui->num_cpu->value());
    settings->setValue("num_gpu", ui->num_gpu->value());
    settings->setValue("gpu_ids", ui->gpu_ids->text());
    settings->setValue("export_pre_script", ui->export_pre_script->path());
    settings->setValue("export_post_script", ui->export_post_script->path());
    settings->setValue("export_custom_script", ui->export_custom_script->path());
    settings->setValue("export_num_processes", ui->export_num_processes->value());
    if(ui->export_copy->isChecked()){
        settings->setValue("export","copy");
    }
    if(ui->export_move->isChecked()){
        settings->setValue("export","move");
    }
    if(ui->export_custom->isChecked()){
        settings->setValue("export","custom");
    }
    settings->beginGroup("Tasks");
    settings->remove("");
    save_task(settings,ui->task_tree->invisibleRootItem());
    settings->endGroup();
}


template<typename SETTINGS>
void load_from_settings(SETTINGS* settings, Ui::SettingsDialog* ui){
    ui->num_cpu->setValue(settings->value("num_cpu").toInt());
    ui->num_gpu->setValue(settings->value("num_gpu").toInt());
    ui->gpu_ids->setText(settings->value("gpu_ids").toString());
    ui->export_pre_script->setPath(settings->value("export_pre_script").toString());
    ui->export_post_script->setPath(settings->value("export_post_script").toString());
    ui->export_custom_script->setPath(settings->value("export_custom_script").toString());
    ui->export_num_processes->setValue(settings->value("export_num_processes").toInt());
    QString export_mode=settings->value("export").toString();
    if(export_mode=="copy"){
        ui->export_copy->setChecked(true);
    }else if(export_mode=="move"){
        ui->export_move->setChecked(true);
    }else if(export_mode=="custom"){
        ui->export_custom->setChecked(true);
    }else{
        ui->export_copy->setChecked(true);
    }
    settings->beginGroup("Tasks");
    ui->task_tree->clear();
    load_task(settings,ui->task_tree->invisibleRootItem());
    settings->endGroup();
    ui->task_tree->expandAll();
    ui->task_tree->resizeColumnToContents(0);

}

}

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog),
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
    QString path = QFileDialog::getOpenFileName(0, "Open configuration file","","Ini files (*.ini);; All files (*)");
    if(! path.isEmpty()){
        QSettings* settings=new QSettings(path,QSettings::IniFormat);
        load_from_settings(settings,ui);
        delete settings;
    }
}

void SettingsDialog::saveToFile()
{
    updateVariables(ui->task_tree->currentItem(),ui->task_tree->currentItem());
    QString path = QFileDialog::getSaveFileName(0, "Save configuration file","","Ini files (*.ini);; All files (*)");
    if(! path.isEmpty()){
        QSettings* settings=new QSettings(path,QSettings::IniFormat);
        save_to_settings(settings,ui);
        delete settings;
    }
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
            old_tree_item->output_variables.append(InputOutputVariable(name,variable,type,is_column));
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

