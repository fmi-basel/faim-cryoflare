#ifndef REMOTEPATHEDIT_H
#define REMOTEPATHEDIT_H

#include <QUrl>
#include "pathedit.h"

class RemotePathEdit : public PathEdit
{
    Q_OBJECT
public:
    RemotePathEdit(QWidget *parent = 0);
    QUrl remotePath() const;
    void setRemotePath(const QUrl &path);

signals:
    void pathChanged(QUrl);
public slots:
    void onRemoteBrowse();

private:
    QPushButton *remote_browse_;
    QUrl remote_path_;
private slots:
    void updateUrl(const QString& text);
};

#endif // REMOTEPATHEDIT_H
