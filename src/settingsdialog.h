#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>
#include <inputoutputvariable.h>

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
    explicit SettingsDialog(QWidget *parent = 0);
    void saveSettings(const QString &path="");
    void loadSettings(const QString &path="");
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
    void updateVariables(QTreeWidgetItem* new_item, QTreeWidgetItem* old_item);
private:
    void saveTask_(QSettings *settings, QTreeWidgetItem *item) const;
    void loadTask_(QSettings *settings, QTreeWidgetItem *parent);
    Ui::SettingsDialog *ui;
    QMenu *task_tree_menu_;
    QAction *task_tree_new_;
    QAction *task_tree_delete_;
    QAction *output_variable_new_;
    QAction *output_variable_delete_;
    QAction *input_variable_new_;
    QAction *input_variable_delete_;
};

#endif // SETTINGSDIALOG_H
