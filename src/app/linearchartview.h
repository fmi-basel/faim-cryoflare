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
#ifndef LINEARCHARTVIEW_H
#define LINEARCHARTVIEW_H

#include "chartview.h"


class LinearChartView : public ChartView
{
Q_OBJECT
public:
    LinearChartView(QWidget *parent = Q_NULLPTR);

signals:
    void indexClicked(int index);

protected:
    virtual void drawSeries_();
    virtual void deselectData_(float start, float end, bool invert=false);
public slots:
    void showDetails(const QPointF &pf, bool state);
    void handleDoubleClicked(const QPointF &pf);
};

#endif // LINEARCHARTVIEW_H
