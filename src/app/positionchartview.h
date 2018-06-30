//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFlare
//
// Copyright (C) 2017-2018 by the CryoFlare Authors
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
// along with CryoFlare.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------------

#ifndef POSITIONCHARTVIEW_H
#define POSITIONCHARTVIEW_H

#include <QGraphicsView>
#include "positionchart.h"

class PositionChartView : public QGraphicsView
{
    Q_OBJECT
public:
    PositionChartView(QWidget* parent);
    virtual void setScene(QGraphicsScene * s);
public slots:
    void enableSelection(bool selection);
signals:
    void clicked(int pos);
protected:
    virtual void resizeEvent(QResizeEvent * event);
    virtual void mousePressEvent(QMouseEvent *event);

private:

};

#endif // POSITIONCHARTVIEW_H
