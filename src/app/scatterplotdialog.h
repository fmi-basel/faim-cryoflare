#ifndef SCATTERPLOTDIALOG_H
#define SCATTERPLOTDIALOG_H

#include <QDialog>
#include "metadatastore.h"

namespace Ui {
class ScatterPlotDialog;
}

class ScatterPlotDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScatterPlotDialog(MetaDataStore * store,QList<InputOutputVariable> result_labels,QWidget *parent = nullptr);
    ~ScatterPlotDialog();
public slots:
    void updateChart();

private:
    Ui::ScatterPlotDialog *ui;
    MetaDataStore * store_;
    QList<InputOutputVariable> result_labels_;
};

#endif // SCATTERPLOTDIALOG_H
