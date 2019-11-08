#ifndef GRIDSQUAREFORM_H
#define GRIDSQUAREFORM_H

#include <QWidget>

namespace Ui {
class GridsquareForm;
}

class GridsquareForm : public QWidget
{
    Q_OBJECT

public:
    explicit GridsquareForm(QWidget *parent = nullptr);
    ~GridsquareForm();

private:
    Ui::GridsquareForm *ui;
};

#endif // GRIDSQUAREFORM_H
