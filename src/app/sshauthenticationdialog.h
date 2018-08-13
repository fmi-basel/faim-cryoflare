#ifndef SSHAUTHENTICATIONDIALOG_H
#define SSHAUTHENTICATIONDIALOG_H

#include <QDialog>
#include <QPair>
#include "../external/qssh/sshconnection.h"

namespace Ui {
class SshAuthenticationDialog;
}

class SshAuthenticationDialog : public QDialog
{
    Q_OBJECT

public:
    typedef QPair<QSsh::SshConnectionParameters::AuthenticationType,QString> auth_type;
    explicit SshAuthenticationDialog( QWidget *parent = nullptr);
    auth_type authentication() const;
    static auth_type getSshAuthentication(const QString& title);
    ~SshAuthenticationDialog();

private:
    Ui::SshAuthenticationDialog *ui;
};

#endif // SSHAUTHENTICATIONDIALOG_H
