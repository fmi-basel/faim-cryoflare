#ifndef DATACHARTFORM_H
#define DATACHARTFORM_H

#include <QWidget>

namespace Ui {
class DataChartForm;
}

class DataChartForm : public QWidget
{
    Q_OBJECT

public:
    explicit DataChartForm(QWidget *parent = nullptr);
    ~DataChartForm();

private:
    Ui::DataChartForm *ui;
};

#endif // DATACHARTFORM_H
