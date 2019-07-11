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

#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QChartView>
#include <QRubberBand>

class ChartView : public QtCharts::QChartView
{
    Q_OBJECT
public:
    ChartView(QWidget *parent = Q_NULLPTR);
public slots:
    void enableSelection(bool selecting);
signals:
    void selected(float start, float end, bool invert=false);
protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    QPointF mapToChartValue_(const QPoint & p) const;
private:
    bool selecting_;
    QRubberBand *rubberband_;
    QPoint rubberband_start_;
};
#endif // CHARTVIEW_H
