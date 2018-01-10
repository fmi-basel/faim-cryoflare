#include <QAction>
#include <QtDebug>
#include "imagetableview.h"

ImageTableView::ImageTableView(QWidget *parent) :
    QTableView(parent),
    select_all_(new QAction("Select all",this)),
    unselect_all_(new QAction("Unselect all",this)),
    select_above_(new QAction("Select above",this)),
    unselect_above_(new QAction("Unselect above",this)),
    select_below_(new QAction("Select below",this)),
    unselect_below_(new QAction("Unselect below",this)),
    invert_selection_(new QAction("Invert selection",this))

{
    setContextMenuPolicy(Qt::ActionsContextMenu);
    addAction(select_all_);
    addAction(unselect_all_);
    addAction(select_above_);
    addAction(unselect_above_);
    addAction(select_below_);
    addAction(unselect_below_);
    addAction(invert_selection_);
    connect(select_all_,SIGNAL(triggered()),this,SLOT(selectEverything()));
    connect(unselect_all_,SIGNAL(triggered()),this,SLOT(unselectEverything()));
    connect(select_above_,SIGNAL(triggered()),this,SLOT(selectAbove()));
    connect(unselect_above_,SIGNAL(triggered()),this,SLOT(unselectAbove()));
    connect(select_below_,SIGNAL(triggered()),this,SLOT(selectBelow()));
    connect(unselect_below_,SIGNAL(triggered()),this,SLOT(unselectBelow()));
    connect(invert_selection_,SIGNAL(triggered()),this,SLOT(invertSelection()));
}

void ImageTableView::selectEverything()
{
    QAbstractItemModel *m=model();
    for(int i=0;i<m->rowCount();++i){
        m->setData(m->index(i,0),Qt::Checked,Qt::CheckStateRole);
    }
}

void ImageTableView::unselectEverything()
{
    QAbstractItemModel *m=model();
    for(int i=0;i<m->rowCount();++i){
        m->setData(m->index(i,0),Qt::Unchecked,Qt::CheckStateRole);
    }
}

void ImageTableView::selectAbove()
{
    QAbstractItemModel *m=model();
    int row=currentIndex().row();
    for(int i=0;i<=row;++i){
        m->setData(m->index(i,0),Qt::Checked,Qt::CheckStateRole);
    }
}

void ImageTableView::unselectAbove()
{
    QAbstractItemModel *m=model();
    int row=currentIndex().row();
    for(int i=0;i<=row;++i){
        m->setData(m->index(i,0),Qt::Unchecked,Qt::CheckStateRole);
    }
}

void ImageTableView::selectBelow()
{
    QAbstractItemModel *m=model();
    int row=currentIndex().row();
    for(int i=row;i<m->rowCount();++i){
        m->setData(m->index(i,0),Qt::Checked,Qt::CheckStateRole);
    }
}

void ImageTableView::unselectBelow()
{
    QAbstractItemModel *m=model();
    int row=currentIndex().row();
    for(int i=row;i<m->rowCount();++i){
        m->setData(m->index(i,0),Qt::Unchecked,Qt::CheckStateRole);
    }
}

void ImageTableView::invertSelection()
{
    QAbstractItemModel *m=model();
    for(int i=0;i<m->rowCount();++i){
        if(m->data(m->index(i,0),Qt::CheckStateRole)==Qt::Checked){
            m->setData(m->index(i,0),Qt::Unchecked,Qt::CheckStateRole);
        }else{
            m->setData(m->index(i,0),Qt::Checked,Qt::CheckStateRole);
       }
    }
}
