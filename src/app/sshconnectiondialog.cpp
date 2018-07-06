#include "sshconnectiondialog.h"
#include "ui_sshconnectiondialog.h"

QSsh::SshConnectionParameters SShConnectionDialog::getConnectionParameter()
{
    SShConnectionDialog dialog;
    int return_code;
    do{
        return_code=dialog.exec();
        QSsh::SshConnectionParameters parameters;
        parameters.host=dialog.host();
        parameters.userName=dialog.user();
        parameters.password=dialog.password();
        QSsh::SshConnection connection(parameters);
        connection.connectToHost();

    }while(return_code!=QDialog::Rejected);
    return QSsh::SshConnectionParameters();

}

SShConnectionDialog::SShConnectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SShConnectionDialog)
{
    ui->setupUi(this);
}

SShConnectionDialog::~SShConnectionDialog()
{
    delete ui;
}

QString SShConnectionDialog::host() const
{
    return ui->host->text();
}

void SShConnectionDialog::setHost(const QString &host)
{
    ui->host->setText(host);
}

QString SShConnectionDialog::user() const
{
    return ui->user->text();
}

void SShConnectionDialog::setUser(const QString &user)
{
    ui->user->setText(user);
}

QString SShConnectionDialog::password() const
{
    return ui->password->text();
}

void SShConnectionDialog::setPassword(const QString &password)
{
    ui->password->setText(password);
}

QString SShConnectionDialog::message() const
{
    return ui->message->text();
}

void SShConnectionDialog::setMessage(const QString &message)
{
    ui->message->setText(message);
}
