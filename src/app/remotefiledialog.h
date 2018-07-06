#ifndef REMOTEFILEDIALOG_H
#define REMOTEFILEDIALOG_H

#include <QDialog>
#include <QPair>
#include "../external/qssh/sftpfilesystemmodel.h"
#include "../external/qssh/sshconnection.h"

namespace Ui {
class RemoteFileDialog;
}

class RemoteFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RemoteFileDialog(QWidget *parent = 0);
    QString path();
    QSsh::SshConnectionParameters connectionParameters();
    static QPair<QSsh::SshConnectionParameters,QString> getRemotePath();
    ~RemoteFileDialog();
public slots:
    void connectToHost();

private:
    Ui::RemoteFileDialog *ui;
    QSsh::SftpFileSystemModel* model_;
    QSsh::SshConnectionParameters ssh_params_;
};

#endif // REMOTEFILEDIALOG_H
