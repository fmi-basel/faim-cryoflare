#ifndef EXPORTPROGRESSDIALOG_H
#define EXPORTPROGRESSDIALOG_H

#include <QDialog>
#include "parallelexporter.h"
namespace Ui {
class ExportProgressDialog;
}

class ExportProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportProgressDialog(QWidget *parent = 0);
    ~ExportProgressDialog();
public slots:
    void start(const QString& title,int num);
    void update(const QList<ExportMessage>& messages,int num_left);
    void update(const QList<ExportMessage>& messages);
    void finish();

private:
    Ui::ExportProgressDialog *ui;
};

#endif // EXPORTPROGRESSDIALOG_H
