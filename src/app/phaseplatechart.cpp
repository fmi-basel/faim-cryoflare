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
#include <QGraphicsPathItem>

#include "phaseplatechart.h"

PhasePlateChart::PhasePlateChart(QWidget *parent):
    QGraphicsView (parent),
    scene_(),
    phase_plates_()
{
    qreal x_size=2.2;
    qreal y_size=1.0;
    qreal y_dist=1.8;
    qreal x_dist=3.4;

    QPainterPath phase_plate_path(QPointF(-x_size*0.5,-y_size*0.5));
    phase_plate_path.arcTo(QRectF(-x_size*0.5-y_size*0.5,-y_size*0.5,y_size,y_size),90,180);
    phase_plate_path.arcTo(QRectF(x_size*0.5-y_size*0.5,-y_size*0.5,y_size,y_size),270,180);
    phase_plate_path.closeSubpath();
    QList<QPointF> phase_plate_positions{QPointF(0,-y_dist*1.5),QPointF(-x_dist*0.5,-y_dist*0.5),QPointF(x_dist*0.5,-y_dist*0.5),QPointF(-x_dist*0.5,y_dist*0.5),QPointF(x_dist*0.5,y_dist*0.5),QPointF(0,y_dist*1.5)};
    for(int i=0;i<phase_plate_positions.size();++i){
        QGraphicsPathItem *item=dynamic_cast<QGraphicsPathItem*>(scene_.addPath(phase_plate_path));
        item->setPos(phase_plate_positions[i]);
        item->setToolTip(QString("%1").arg(i+1));
        item->setZValue(i+1);
        item->setFlag(QGraphicsItem::ItemIsSelectable,true);
        phase_plates_.append(item);
    }
    int num_rows=4;
    int num_columns=19;
    qreal pos_size=0.1;
    qreal post_dist=0.12;
    QPainterPath phase_plate_pos_path;
    phase_plate_pos_path.addEllipse(-pos_size*0.5,-pos_size*0.5,pos_size,pos_size);
    for(int row=0;row<num_rows;++row){
        for(int column=0;column<num_columns;++column){
            QGraphicsPathItem *item=dynamic_cast<QGraphicsPathItem*>(scene_.addPath(phase_plate_pos_path));

        }
    }
}
