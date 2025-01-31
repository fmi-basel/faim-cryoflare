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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include "settings.h"
#include "metadatastore.h"
#include <LimeReport>
#include <inputoutputvariable.h>
#include "task.h"

//fw decl
class QMenu;
class QAction;
class QTreeWidgetItem;

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(MetaDataStore* meta_data_store,TaskConfiguration* task_config, QWidget *parent = nullptr);
    void saveSettings();
    void loadSettings();
    ~SettingsDialog();
public slots:
    void newTask();
    void deleteTask();
    void newOutputVariable(const InputOutputVariable& variable=InputOutputVariable());
    void deleteOutputVariable();
    void newInputVariable(const InputOutputVariable& variable=InputOutputVariable());
    void deleteInputVariable();
    void loadFromFile();
    void saveToFile();
    void saveAsDefaults();
    void resetToDefaults();
    void updateVariables(QTreeWidgetItem* new_item, QTreeWidgetItem* old_item);
    void designReport();
private:
    Ui::SettingsDialog *ui;
    MetaDataStore* meta_data_store_;
    TaskConfiguration* task_config_;
    QMenu *task_tree_menu_;
    QAction *task_tree_new_;
    QAction *task_tree_delete_;
    QAction *output_variable_new_;
    QAction *output_variable_delete_;
    QAction *input_variable_new_;
    QAction *input_variable_delete_;
};

#endif // SETTINGSDIALOG_H
