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

#ifndef IMAGETABLEVIEW_H
#define IMAGETABLEVIEW_H

#include <QTableView>

class ImageTableView : public QTableView
{
    Q_OBJECT
public:
    explicit ImageTableView(QWidget *parent = nullptr);
    QAction *selectAllAction() const;
    QAction *unselectAllAction() const;
    QAction *selectAboveAction() const;
    QAction *unselectAboveAction() const;
    QAction *selectBelowAction() const;
    QAction *unselectBelowAction() const;
    QAction *invertSelectionAction() const;

signals:

public slots:
    void selectEverything();
    void unselectEverything();
    void selectAbove();
    void unselectAbove();
    void selectBelow();
    void unselectBelow();
    void invertSelection();

protected:
    QAction *select_all_;
    QAction *unselect_all_;
    QAction *select_above_;
    QAction *unselect_above_;
    QAction *select_below_;
    QAction *unselect_below_;
    QAction *invert_selection_;

};

#endif // IMAGETABLEVIEW_H
