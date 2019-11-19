#ifndef GRIDSQUAREFORM_H
#define GRIDSQUAREFORM_H

#include "gridsquaretablemodel.h"
#include "gradient.h"
#include <QWidget>

//fw decl
class TaskConfiguration;

namespace Ui {
class GridsquareForm;
}

class GridsquareForm : public QWidget
{
    Q_OBJECT

public:
    explicit GridsquareForm(QWidget *parent = nullptr);
    ~GridsquareForm();
    void init(MetaDataStore* meta_data_store, TaskConfiguration* task_config);
public slots:
    void loadGridsquare();
    void updateMarkers();
    void updateResultLabels();

private:
    Ui::GridsquareForm *ui;
    MetaDataStore* meta_data_store_;
    GridsquareTableModel *gridsquare_model_;
    TaskConfiguration* task_configuration_;
    Gradient gradient_;
};

#endif // GRIDSQUAREFORM_H
