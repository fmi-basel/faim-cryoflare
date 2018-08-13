#include "sshauthenticationdialog.h"
#include "ui_sshauthenticationdialog.h"

SshAuthenticationDialog::SshAuthenticationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SshAuthenticationDialog)
{
    ui->setupUi(this);
}

SshAuthenticationDialog::auth_type SshAuthenticationDialog::authentication() const
{
    if(ui->rb_password->isChecked()){
        return auth_type(QSsh::SshConnectionParameters::AuthenticationByPassword,ui->password->text());
    }else{
        return auth_type(QSsh::SshConnectionParameters::AuthenticationByKey,ui->key_path->path());
    }
}

SshAuthenticationDialog::auth_type SshAuthenticationDialog::getSshAuthentication(const QString &title)
{
    SshAuthenticationDialog dialog;
    dialog.setWindowTitle(title);
    if(dialog.exec()==QDialog::Accepted){
        return dialog.authentication();
    }
    return SshAuthenticationDialog::auth_type();

}

SshAuthenticationDialog::~SshAuthenticationDialog()
{
    delete ui;
}
