//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2017-2018 by the CryoFLARE Authors
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3.0 of the License.
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License
// along with CryoFLARE.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------------

#include <QAction>
#include <QtDebug>
#include <QHeaderView>
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
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    horizontalHeader()->setResizeContentsPrecision(0);
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

QAction *ImageTableView::invertSelectionAction() const
{
    return invert_selection_;
}

QAction *ImageTableView::unselectBelowAction() const
{
    return unselect_below_;
}

QAction *ImageTableView::selectBelowAction() const
{
    return select_below_;
}

QAction *ImageTableView::unselectAboveAction() const
{
    return unselect_above_;
}

QAction *ImageTableView::selectAboveAction() const
{
    return select_above_;
}

QAction *ImageTableView::unselectAllAction() const
{
    return unselect_all_;
}

QAction *ImageTableView::selectAllAction() const
{
    return select_all_;
}
