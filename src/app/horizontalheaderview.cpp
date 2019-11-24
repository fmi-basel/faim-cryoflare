//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2019 by the CryoFLARE Authors
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
#include "horizontalheaderview.h"
#include <QDebug>
#include <QScrollBar>
HorizontalHeaderView::HorizontalHeaderView( QWidget * parent):
    QHeaderView(Qt::Horizontal,parent),
    sibling_()
{
    setSectionResizeMode (QHeaderView::Fixed);
}

void HorizontalHeaderView::setSibling(QTableView *sibling)
{
    connect(sibling->horizontalHeader(),&QHeaderView::sectionResized,this,&HorizontalHeaderView::siblingSectionResized);
    connect(sibling->horizontalScrollBar(),&QAbstractSlider::valueChanged,this,&HorizontalHeaderView::scrolled);
    sibling_=sibling;
}

void HorizontalHeaderView::siblingSectionResized(int logicalIndex, int /*oldSize*/, int newSize)
{
    reset();
    QHeaderView::resizeSection(logicalIndex,newSize);
    scrolled(sibling_->horizontalScrollBar()->value());
}

void HorizontalHeaderView::scrolled(int /*logical_index*/)
{
    setOffset(-1-sibling_->verticalHeader()->geometry().width()-sibling_->horizontalHeader()->sectionViewportPosition(0));
}

