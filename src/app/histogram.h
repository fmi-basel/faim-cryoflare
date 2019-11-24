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
#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <QPointF>
#include <QVector>



class Histogram
{
public:
    Histogram(float start, float end, int bins);
    void add(float value);
    void add(const QVector<float>& values);
    void remove(float value);
    void remove(const QVector<float>& values);
    float start() const;
    float end() const;
    QVector<int> data() const;
    QList<QPointF> dataPoints() const;
    float width() const;

protected:
    float start_;
    float width_;
    QVector<int> bins_;
};

#endif // HISTOGRAM_H
