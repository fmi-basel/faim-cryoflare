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

#ifndef POSITIONCHART_H
#define POSITIONCHART_H

#include<QGraphicsScene>
#include <QGradient>
#include<QVector>
#include<QPainterPath>

class PositionChart : public QGraphicsScene
{
public:
    PositionChart(QObject *parent = Q_NULLPTR);
    void addPositions(const QPainterPath & path=QPainterPath(), const QHash<int,QPointF>& pos=QHash<int,QPointF>(), bool back=false);
    void setMinMaxValue(double minval, double maxval);
    void setValues(const QHash<int,double> &values);
    void clear();
    QColor colorAt(double value);
    QGraphicsSimpleTextItem* min_label;
    QGraphicsSimpleTextItem* max_label;
private:
    QHash<int,QGraphicsPathItem*> items_;
    QGradientStops gradient_stops_;
    double minval_;
    double valrange_;
};

#endif // POSITIONCHART_H
