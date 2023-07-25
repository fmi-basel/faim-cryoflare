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

#ifndef IMAGETABLEVIEW_H
#define IMAGETABLEVIEW_H

#include <QTableView>

//fw decl
class QAbstractItemModel;

class ImageTableView : public QTableView
{
    Q_OBJECT
public:
    explicit ImageTableView(QWidget *parent = nullptr);
    virtual void setModel(QAbstractItemModel *m);
    QAction *selectAllAction() const;
    QAction *deselectAllAction() const;
    QAction *selectAboveAction() const;
    QAction *deselectAboveAction() const;
    QAction *selectBelowAction() const;
    QAction *deselectBelowAction() const;
    QAction *invertSelectionAction() const;

signals:

public slots:
    void selectEverything();
    void deselectEverything();
    void selectAbove();
    void deselectAbove();
    void selectBelow();
    void deselectBelow();
    void invertSelection();
    void updateColumnVisibility();
    void jumpToMicrograph(int index);

protected:
    QAction *select_all_;
    QAction *deselect_all_;
    QAction *select_above_;
    QAction *deselect_above_;
    QAction *select_below_;
    QAction *deselect_below_;
    QAction *invert_selection_;

};

#endif // IMAGETABLEVIEW_H
