#ifndef MICROGRAPHSFORM_H
#define MICROGRAPHSFORM_H

#include <QWidget>

namespace Ui {
class MicrographsForm;
}

class MicrographsForm : public QWidget
{
    Q_OBJECT

public:
    explicit MicrographsForm(QWidget *parent = nullptr);
    ~MicrographsForm();

private:
    Ui::MicrographsForm *ui;
};

#endif // MICROGRAPHSFORM_H
