//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2019 by the CryoFLARE Authors
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3.0 of the License.
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License
// along with CryoFLARE.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------------
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
