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
    browseRemoteData(const QString& message="");
    browseRemoteRawData(const QString& message="");

private:
    Ui::ExportDialog *ui;
    QProgressDialog* progress_dialog_;
};

#endif // EXPORTDIALOG_H
