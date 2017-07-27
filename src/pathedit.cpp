#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include "pathedit.h"


PathEdit::PathEdit(QWidget *parent):
    QWidget(parent),
    path_type_(OpenFileName),
    caption_(),
    path_(),
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
    path_(path),
    filter_(filter),
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
    QString path;
    switch(path_type_){
    case ExistingDirectory:
        path = QFileDialog::getExistingDirectory(0, caption_,path_,  QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);
        break;
    case OpenFileName:
        path = QFileDialog::getOpenFileName(0, caption_,path_,filter_);
        break;
    case SaveFileName:
        path = QFileDialog::getSaveFileName(0, caption_,path_,filter_);
        break;
    }
    if(! path.isEmpty()){
        path_widget_->setText(path);
    }
}
