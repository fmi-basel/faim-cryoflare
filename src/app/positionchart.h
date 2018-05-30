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

#ifndef POSITIONCHART_H
#define POSITIONCHART_H

#include<QGraphicsScene>
#include <QGradient>
#include<QVector>

class PositionChart : public QGraphicsScene
{
public:
    PositionChart(QObject *parent = Q_NULLPTR);
    void addPositions(const QPainterPath & path=QPainterPath(), const QList<QPointF>& pos=QList<QPointF>());
    void setMinMaxValue(float minval, float maxval);
    void setValues(const QVector<float> &values);
    QColor colorAt(float value);
private:
    QVector<QGraphicsPathItem*> items_;
    QGradientStops gradient_stops_;
    float minval_;
    float valrange_;
    QGraphicsTextItem* min_label_;
    QGraphicsTextItem* max_label_;
};

#endif // POSITIONCHART_H