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
    void setName(const QString &name);
    void setScript(const QString& script);
    void setGpu(bool gpu);
    QList<InputOutputVariable> input_variables;
    QList<InputOutputVariable> output_variables;

signals:

public slots:
    void onPathBrowse();
private:
    void init_();
    PathEdit *path_widget_;
    QCheckBox *gpu_check_box_;
};

#endif // TASKTREEWIDGETITEM_H
