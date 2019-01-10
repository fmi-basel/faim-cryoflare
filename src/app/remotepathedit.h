#ifndef REMOTEPATHEDIT_H
#define REMOTEPATHEDIT_H

#include "sftpurl.h"
#include "pathedit.h"

class RemotePathEdit : public PathEdit
{
    Q_OBJECT
public:
    RemotePathEdit(QWidget *parent = 0);
    SftpUrl remotePath() const;
    void setRemotePath(const SftpUrl &path);

signals:
    void pathChanged(SftpUrl);
public slots:
    void onRemoteBrowse();

private:
    QPushButton *remote_browse_;
    SftpUrl remote_path_;
private slots:
    void updateUrl(const QString& text);
};

#endif // REMOTEPATHEDIT_H
