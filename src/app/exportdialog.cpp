#include "exportdialog.h"
#include "ui_exportdialog.h"
#include "sshconnectiondialog.h"

ExportDialog::ExportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportDialog),
    progress_dialog_(new QProgressDialog("Connecting...", "Abort", 0, 0, this))
{
    ui->setupUi(this);
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

ExportDialog::browseRemoteData(const QString &message)
{
    SShConnectionDialog dialog;
    dialog.setMessage(message);
    if(dialog.exec()==QDialog::Accepted){

        QSsh::SshConnectionParameters parameters;
        parameters.host=dialog.host();
        parameters.userName=dialog.user();
        parameters.password=dialog.password();
        QSsh::SshConnection connection(parameters);
        connection.connectToHost();
        progress_dialog_->show();

    }
    return QSsh::SshConnectionParameters();


}

ExportDialog::browseRemoteRawData(const QString &message)
{

}
