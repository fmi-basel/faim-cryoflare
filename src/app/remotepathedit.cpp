#include <QPushButton>
#include <QIcon>
#include <QLayout>
#include <QLineEdit>
#include "remotepathedit.h"
#include "remotefiledialog.h"

RemotePathEdit::RemotePathEdit(QWidget *parent):
    PathEdit(parent),
    remote_browse_(new QPushButton(QIcon(":/icons/document-open-remote.png"),"",this)),
    remote_path_()
{
    layout()->addWidget(remote_browse_);
    connect(remote_browse_, &QPushButton::clicked,this,&RemotePathEdit::onRemoteBrowse);
    connect(path_widget_,&QLineEdit::textChanged,this,&RemotePathEdit::updateUrl);
}

QUrl RemotePathEdit::remotePath() const
{
    return remote_path_;
}

void RemotePathEdit::setRemotePath(const QUrl &path)
{
    remote_path_=path;
}

void RemotePathEdit::onRemoteBrowse()
{
    // todo implement path_types for remote locations ( probably in SftpFilesystemModel)
    switch(path_type_){
    case ExistingDirectory:
        break;
    case OpenFileName:
        break;
    case SaveFileName:
        break;
    }


    QUrl new_path=RemoteFileDialog::getRemotePath(remote_path_);
    if(new_path.isValid()){
        path_widget_->setText(new_path.toString(QUrl::RemovePassword));
        remote_path_=new_path;
    }

}

void RemotePathEdit::updateUrl(const QString &text)
{
    QUrl new_url(text);
    if(new_url.isValid()){
        new_url.setPassword(remote_path_.password());
        remote_path_=new_url;
    }
}
