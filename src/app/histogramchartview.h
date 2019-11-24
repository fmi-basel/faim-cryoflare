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
#ifndef HISTOGRAMCHARTVIEW_H
#define HISTOGRAMCHARTVIEW_H

#include "chartview.h"



class HistogramChartView : public ChartView
{
public:
    HistogramChartView(QWidget *parent = Q_NULLPTR);
protected:
    virtual void drawSeries_();
    virtual void deselectData_(float start, float end, bool invert=false);
    virtual void mouseReleaseEvent(QMouseEvent *event);
};

#endif // HISTOGRAMCHARTVIEW_H
