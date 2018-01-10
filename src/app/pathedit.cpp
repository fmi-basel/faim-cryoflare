#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include "pathedit.h"


PathEdit::PathEdit(QWidget *parent):
    QWidget(parent),
    path_type_(OpenFileName),
    caption_(),
    filter_(),
    path_widget_(new QLineEdit(this)),
    browse_(new QPushButton("Browse...",this))
{
    QHBoxLayout *layout=new QHBoxLayout();
    layout->addWidget(path_widget_);
    layout->addWidget(browse_);
    this->setLayout(layout);
    connect(browse_,SIGNAL(clicked()),this,SLOT(onBrowse()));
    connect(path_widget_,SIGNAL(textChanged(QString)),this,SIGNAL(pathChanged(QString)));
}

PathEdit::PathEdit(PathEdit::PathType t, QString caption, QString path, QString filter, QWidget *parent):
    QWidget(parent),
    path_type_(t),
    caption_(caption),
    filter_(filter),
    path_widget_(new QLineEdit(path,this)),
    browse_(new QPushButton("Browse...",this))
{
    QHBoxLayout *layout=new QHBoxLayout();
    layout->addWidget(path_widget_);
    layout->addWidget(browse_);
    this->setLayout(layout);
    connect(browse_,SIGNAL(clicked()),this,SLOT(onBrowse()));
    connect(path_widget_,SIGNAL(textChanged(QString)),this,SIGNAL(pathChanged(QString)));
}

QString PathEdit::path() const
{
    return path_widget_->text();
}

void PathEdit::setPath(const QString &path)
{
    path_widget_->setText(path);
}

void PathEdit::onBrowse()
{
    QString new_path;
    switch(path_type_){
    case ExistingDirectory:
        new_path = QFileDialog::getExistingDirectory(0, caption_,path(),  QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);
        break;
    case OpenFileName:
        new_path = QFileDialog::getOpenFileName(0, caption_,path(),filter_);
        break;
    case SaveFileName:
        new_path = QFileDialog::getSaveFileName(0, caption_,path(),filter_);
        break;
    }
    if(! new_path.isEmpty()){
        path_widget_->setText(new_path);
    }
}
