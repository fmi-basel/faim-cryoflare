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
#include <algorithm>
#include "gradient.h"

Gradient::Gradient(const QMap<qreal,QColor> &stops):
    stops_(stops)
{
}

QList<QColor> Gradient::createColors(const QList<qreal>& values) const
{
    QList<QColor> result;
    QList<qreal> positions=stops_.keys();
    foreach(qreal value, values){
        if(qIsNaN(value)){
            result.append(QColor::fromRgbF(1.0,1.0,1.0,1.0));
            continue;
        }
        QList<qreal>::iterator lower_bound=std::lower_bound (positions.begin(), positions.end(), value);
        if(lower_bound==positions.begin()){
            result.append(stops_.first());
            continue;
        }
        if(lower_bound==positions.end()){
            result.append(stops_.last());
            continue;
        }
        qreal upper_pos=*lower_bound;
        --lower_bound;
        qreal lower_pos=*lower_bound;
        QColor lower_color=stops_.value(lower_pos);
        QColor upper_color=stops_.value(upper_pos);
        qreal interval=upper_pos-lower_pos;
        qreal rel_offset=(value-lower_pos)/interval;
        qreal irel_offset=1.0-rel_offset;
        qreal red=std::max(0.0,std::min(1.0,lower_color.redF()*irel_offset+upper_color.redF()*rel_offset));
        qreal green=std::max(0.0,std::min(1.0,lower_color.greenF()*irel_offset+upper_color.greenF()*rel_offset));
        qreal blue=std::max(0.0,std::min(1.0,lower_color.blueF()*irel_offset+upper_color.blueF()*rel_offset));
        qreal alpha=std::max(0.0,std::min(1.0,lower_color.alphaF()*irel_offset+upper_color.alphaF()*rel_offset));
        result.append(QColor::fromRgbF(red,green,blue,alpha));
    }
    return result;
}

QMap<qreal, QColor> Gradient::stops()
{
    return stops_;
}
