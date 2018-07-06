#ifndef SSHCONNECTIONDIALOG_H
#define SSHCONNECTIONDIALOG_H

#include <QDialog>
#include "../external/qssh/sshconnection.h"
namespace Ui {
class SShConnectionDialog;
}

class SShConnectionDialog : public QDialog
{
    Q_OBJECT

public:
    static QSsh::SshConnectionParameters getConnectionParameter();
    explicit SShConnectionDialog(QWidget *parent = 0);
    ~SShConnectionDialog();
    QString host() const;
    void setHost(const QString& host);
    QString user() const;
    void setUser(const QString& user);
    QString password() const;
    void setPassword(const QString& password);
    QString message() const;
    void setMessage(const QString&  message);
private:
    Ui::SShConnectionDialog *ui;
};

#endif // SSHCONNECTIONDIALOG_H
