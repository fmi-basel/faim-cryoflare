#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include <QProgressDialog>
#include "../external/qssh/sshconnection.h"

namespace Ui {
class ExportDialog;
}

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(QWidget *parent = 0);
    ~ExportDialog();
private:
    Ui::ExportDialog *ui;
};

#endif // EXPORTDIALOG_H
