//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2017-2019 by the CryoFLARE Authors
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
#include <QAbstractProxyModel>
#include <QHeaderView>
#include "imagetableview.h"
#include "imagetablemodel.h"

ImageTableView::ImageTableView(QWidget *parent) :
    QTableView(parent),
    select_all_(new QAction("Select all",this)),
    deselect_all_(new QAction("Deselect all",this)),
    select_above_(new QAction("Select above",this)),
    deselect_above_(new QAction("Deselect above",this)),
    select_below_(new QAction("Select below",this)),
    deselect_below_(new QAction("Deselect below",this)),
    invert_selection_(new QAction("Invert selection",this))

{
    setContextMenuPolicy(Qt::ActionsContextMenu);
    addAction(select_all_);
    addAction(deselect_all_);
    addAction(select_above_);
    addAction(deselect_above_);
    addAction(select_below_);
    addAction(deselect_below_);
    addAction(invert_selection_);
    connect(select_all_,SIGNAL(triggered()),this,SLOT(selectEverything()));
    connect(deselect_all_,SIGNAL(triggered()),this,SLOT(deselectEverything()));
    connect(select_above_,SIGNAL(triggered()),this,SLOT(selectAbove()));
    connect(deselect_above_,SIGNAL(triggered()),this,SLOT(deselectAbove()));
    connect(select_below_,SIGNAL(triggered()),this,SLOT(selectBelow()));
    connect(deselect_below_,SIGNAL(triggered()),this,SLOT(deselectBelow()));
    connect(invert_selection_,SIGNAL(triggered()),this,SLOT(invertSelection()));
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    horizontalHeader()->setResizeContentsPrecision(0);
}

void ImageTableView::setModel(QAbstractItemModel *m)
{
    if(model()){
        disconnect(model(),&QAbstractItemModel::modelReset,this,&ImageTableView::updateColumnVisibility);
    }
    QTableView::setModel(m);
    connect(model(),&QAbstractItemModel::modelReset,this,&ImageTableView::updateColumnVisibility);
}

void ImageTableView::selectEverything()
{
    QSet<int> rows;
    QAbstractProxyModel* proxy_model=qobject_cast<QAbstractProxyModel*>(model());
    ImageTableModel *m(nullptr);
    if(proxy_model){
        m=qobject_cast<ImageTableModel*>(proxy_model->sourceModel());
    }else{
        m=qobject_cast<ImageTableModel*>(model());
    }
    if(!m){
        throw "Invalid ImageTableModel";
    }
    for(int i=0;i<m->rowCount();++i){
        rows.insert(i);
    }
    m->setExports(rows,true);
}

void ImageTableView::deselectEverything()
{
    QSet<int> rows;
    QAbstractProxyModel* proxy_model=qobject_cast<QAbstractProxyModel*>(model());
    ImageTableModel *m(nullptr);
    if(proxy_model){
        m=qobject_cast<ImageTableModel*>(proxy_model->sourceModel());
    }else{
        m=qobject_cast<ImageTableModel*>(model());
    }
    if(!m){
        throw "Invalid ImageTableModel";
    }
    for(int i=0;i<m->rowCount();++i){
        rows.insert(i);
    }
    m->setExports(rows,false);
}

void ImageTableView::selectAbove()
{
    QSet<int> rows;
    QAbstractProxyModel* proxy_model=qobject_cast<QAbstractProxyModel*>(model());
    ImageTableModel *m(nullptr);
    if(proxy_model){
        m=qobject_cast<ImageTableModel*>(proxy_model->sourceModel());
    }else{
        m=qobject_cast<ImageTableModel*>(model());
    }
    if(!m){
        throw "Invalid ImageTableModel";
    }
    int row=currentIndex().row();
    for(int i=0;i<=row;++i){
        if(proxy_model){
            rows.insert(proxy_model->mapToSource(proxy_model->index(i,0)).row());
        }else{
            rows.insert(i);
        }
    }
    m->setExports(rows,true);
}

void ImageTableView::deselectAbove()
{
    QSet<int> rows;
    QAbstractProxyModel* proxy_model=qobject_cast<QAbstractProxyModel*>(model());
    ImageTableModel *m(nullptr);
    if(proxy_model){
        m=qobject_cast<ImageTableModel*>(proxy_model->sourceModel());
    }else{
        m=qobject_cast<ImageTableModel*>(model());
    }
    if(!m){
        throw "Invalid ImageTableModel";
    }
    int row=currentIndex().row();
    for(int i=0;i<=row;++i){
        if(proxy_model){
            rows.insert(proxy_model->mapToSource(proxy_model->index(i,0)).row());
        }else{
            rows.insert(i);
        }
    }
    m->setExports(rows,false);
}

void ImageTableView::selectBelow()
{
    QSet<int> rows;
    QAbstractProxyModel* proxy_model=qobject_cast<QAbstractProxyModel*>(model());
    ImageTableModel *m(nullptr);
    if(proxy_model){
        m=qobject_cast<ImageTableModel*>(proxy_model->sourceModel());
    }else{
        m=qobject_cast<ImageTableModel*>(model());
    }
    if(!m){
        throw "Invalid ImageTableModel";
    }
    int row=currentIndex().row();
    for(int i=row;i<m->rowCount();++i){
        if(proxy_model){
            rows.insert(proxy_model->mapToSource(proxy_model->index(i,0)).row());
        }else{
            rows.insert(i);
        }
    }
    m->setExports(rows,true);
}

void ImageTableView::deselectBelow()
{
    QSet<int> rows;
    QAbstractProxyModel* proxy_model=qobject_cast<QAbstractProxyModel*>(model());
    ImageTableModel *m(nullptr);
    if(proxy_model){
        m=qobject_cast<ImageTableModel*>(proxy_model->sourceModel());
    }else{
        m=qobject_cast<ImageTableModel*>(model());
    }
    if(!m){
        throw "Invalid ImageTableModel";
    }
    int row=currentIndex().row();
    for(int i=row;i<m->rowCount();++i){
        if(proxy_model){
            rows.insert(proxy_model->mapToSource(proxy_model->index(i,0)).row());
        }else{
            rows.insert(i);
        }
    }
    m->setExports(rows,false);
}

void ImageTableView::invertSelection()
{
    QAbstractProxyModel* proxy_model=qobject_cast<QAbstractProxyModel*>(model());
    ImageTableModel *m(nullptr);
    if(proxy_model){
        m=qobject_cast<ImageTableModel*>(proxy_model->sourceModel());
    }else{
        m=qobject_cast<ImageTableModel*>(model());
    }
    if(!m){
        throw "Invalid ImageTableModel";
    }
    QSet<int> select_rows;
    QSet<int> deselect_rows;
    for(int i=0;i<m->rowCount();++i){
        if(m->data(m->index(i,0),Qt::CheckStateRole)==Qt::Checked){
            deselect_rows.insert(i);
        }else{
            select_rows.insert(i);
       }
    }
    m->setExports(select_rows,true);
    m->setExports(deselect_rows,false);
}

void ImageTableView::updateColumnVisibility()
{
    for(int i=0;i<model()->columnCount();++i){
        setColumnHidden(i,! model()->headerData(i,Qt::Horizontal,ImageTableModel::VisibilityRole).toBool());
    }
}

QAction *ImageTableView::invertSelectionAction() const
{
    return invert_selection_;
}

QAction *ImageTableView::deselectBelowAction() const
{
    return deselect_below_;
}

QAction *ImageTableView::selectBelowAction() const
{
    return select_below_;
}

QAction *ImageTableView::deselectAboveAction() const
{
    return deselect_above_;
}

QAction *ImageTableView::selectAboveAction() const
{
    return select_above_;
}

QAction *ImageTableView::deselectAllAction() const
{
    return deselect_all_;
}

QAction *ImageTableView::selectAllAction() const
{
    return select_all_;
}
void ImageTableView::jumpToMicrograph(int row)
{
   QModelIndex new_index=model()->index(row,currentIndex().column());
   setCurrentIndex(new_index);
   scrollTo(new_index);  
}

