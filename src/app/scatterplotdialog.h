#ifndef SCATTERPLOTDIALOG_H
#define SCATTERPLOTDIALOG_H

#include <QDialog>
#include "imagetablemodel.h"

namespace Ui {
class ScatterPlotDialog;
}

class ScatterPlotDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScatterPlotDialog(ImageTableModel * model,QWidget *parent = 0);
    ~ScatterPlotDialog();
public slots:
    void updateChart();

private:
    Ui::ScatterPlotDialog *ui;
    ImageTableModel * model_;
};

#endif // SCATTERPLOTDIALOG_H
