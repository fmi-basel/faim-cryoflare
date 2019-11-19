#include "exportprogressdialog.h"
#include "ui_exportprogressdialog.h"

ExportProgressDialog::ExportProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportProgressDialog)
{
    ui->setupUi(this);
    setModal(true);
    hide();
}

ExportProgressDialog::~ExportProgressDialog()
{
    delete ui;
}

void ExportProgressDialog::start(const QString &title, int num)
{
    setWindowTitle(title);
    ui->progress->reset();
    ui->progress->setMaximum(num);
    ui->finish->hide();
    ui->cancel->show();
    ui->details->clear();
    open();
}

void ExportProgressDialog::update(const QList<ExportMessage> &messages, int num_left)
{
    ui->progress->setValue(ui->progress->maximum()-num_left);
    update(messages);
}

void ExportProgressDialog::update(const QList<ExportMessage> &messages)
{
    QString new_messages;
    foreach( ExportMessage m,messages){
        if(m.type==ExportMessage::ERROR){
            new_messages+=QString("<font color=red><b>%1: %2</b></font><br>").arg(m.id).arg(m.text);
        }else{
            new_messages+=QString("%1: %2<br>").arg(m.id).arg(m.text);
        }
    }
    ui->details->setText(ui->details->text()+new_messages);
}

void ExportProgressDialog::finish()
{
    ui->finish->show();
    ui->cancel->hide();
}
