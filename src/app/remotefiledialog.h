#ifndef REMOTEFILEDIALOG_H
#define REMOTEFILEDIALOG_H

#include <QDialog>
#include "../external/qssh/sftpfilesystemmodel.h"

namespace Ui {
class RemoteFileDialog;
}

class RemoteFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RemoteFileDialog(QWidget *parent = 0);
    QString path();
    static QString getRemotePath();
    ~RemoteFileDialog();

private:
    Ui::RemoteFileDialog *ui;
    QSsh::SftpFileSystemModel* model_;
};

#endif // REMOTEFILEDIALOG_H
