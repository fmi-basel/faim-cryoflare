#ifndef REMOTEFILEDIALOG_H
#define REMOTEFILEDIALOG_H

#include <QDialog>
#include <QUrl>

//fw decl
namespace QSsh {
class SftpFileSystemModel;
} //ns Qssh

namespace Ui {
class RemoteFileDialog;
}

class RemoteFileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RemoteFileDialog(const QUrl& remote_path=QUrl(),QWidget *parent = 0);
    QUrl remotePath() const;
    static QUrl getRemotePath(const QUrl& path );
    ~RemoteFileDialog();
public slots:
    void connectToHost(bool con);
    void onConnectionEstablished();
    void onConnectionError(const QString &error);

private:
    Ui::RemoteFileDialog *ui;
    QSsh::SftpFileSystemModel* model_;
    QUrl remote_path_;
};

#endif // REMOTEFILEDIALOG_H
