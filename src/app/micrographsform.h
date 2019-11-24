#ifndef MICROGRAPHSFORM_H
#define MICROGRAPHSFORM_H

#include "imagetablemodel.h"
#include "imagetablesortfilterproxymodel.h"
#include "metadatastore.h"
#include "positionchart.h"
#include "settings.h"
#include "tablesummarymodel.h"
#include "micrographprocessor.h"
#include <QFormLayout>
#include <QMainWindow>

//fw decl
class TaskConfiguration;

namespace Ui {
class MicrographsForm;
}

class MicrographsForm : public QWidget
{
    Q_OBJECT

public:
    explicit MicrographsForm(QMainWindow *parent=nullptr);
    void init(QMainWindow *parent,MetaDataStore* store,MicrographProcessor* processor,TaskConfiguration * task_config);
    ~MicrographsForm();

public slots:
    void updateDetailsFromModel(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void updateDetailsfromView();
    void updateTaskWidgets();
    void updatePhasePlateChart();
    void inputDataChanged();
    void reprocessCurrent();
    void reprocessSelected();
    void reprocessAll();
    void phasePlateSelectionChanged();
    void phasePlateSelectionFinished(QRect rubberBandRect, QPointF fromScenePoint, QPointF toScenePoint);
protected:
    virtual void changeEvent(QEvent *event);

private:
    void updateDetails_();
    void updateTaskWidget_(Settings *settings, QFormLayout *parent_input_layout, QFormLayout *parent_output_layout);
    Ui::MicrographsForm *ui;
    MetaDataStore* meta_data_store_;
    MicrographProcessor* processor_;
    ImageTableModel *model_;
    ImageTableSortFilterProxyModel *sort_proxy_;
    TableSummaryModel* summary_model_;
    QTimer chart_update_timer_;
    double histogram_min_;
    double histogram_bucket_size_;
    QVector<double> histogram_;
    PositionChart* phase_plate_chart_;
    PositionChart* phase_plate_position_chart_;
    int phase_plate_level_;
    int current_phase_plate_;
    QMenu* micrograph_menu_;
    QMenu* chart_menu_;
};

#endif // MICROGRAPHSFORM_H
