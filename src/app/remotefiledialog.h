#ifndef REMOTEFILEDIALOG_H
#define REMOTEFILEDIALOG_H

#include <QDialog>
#include <QSortFilterProxyModel>
#include <sftpurl.h>

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
    explicit RemoteFileDialog(const SftpUrl& remote_path=SftpUrl(),QWidget *parent = nullptr);
    SftpUrl remotePath() const;
    static SftpUrl getRemotePath(const SftpUrl& path );
    ~RemoteFileDialog();
public slots:
    void connectToHost(bool con);
    void onConnectionEstablished();
    void onConnectionError(const QString &error);
protected slots:
    void modelReady_();
private:
    void initSftpFileSystemModel_();
    Ui::RemoteFileDialog *ui;
    QSsh::SftpFileSystemModel* model_;
    QSortFilterProxyModel* proxy_;
    SftpUrl remote_path_;
    QStringList initial_path_;
    QModelIndex initial_idx_;
};

#endif // REMOTEFILEDIALOG_H
