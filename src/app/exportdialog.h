#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include <QProgressDialog>

namespace Ui {
class ExportDialog;
}

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(QWidget *parent = 0);
    ~ExportDialog();
public slots:
    void browseRemoteData();
    void browseRemoteRawData();

private:
    Ui::ExportDialog *ui;
    QProgressDialog* progress_dialog_;
};

#endif // EXPORTDIALOG_H
