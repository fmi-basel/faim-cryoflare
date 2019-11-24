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
#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QGraphicsView>
#include <QGraphicsScene>

class ImageViewer : public QGraphicsView
{
public:
    ImageViewer(QWidget *parent = nullptr);
public slots:
    void load(const QString& path);
    void addMarker(const QPointF& pos, qreal r, const QString& tooltip, const QColor & color);
    void clear();
    void clearMarkers();
    void setLegend(const QLinearGradient& gradient, const QString& lmin,const QString& lmax);
protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
    virtual void drawForeground(QPainter * painter,const QRectF& rect);
    QGraphicsScene scene_;
    QGraphicsPixmapItem* pixmap_item_;
    QList<QGraphicsItem*> markers_;
    int scalefactor_;
    QLinearGradient legend_gradient_;
    QString legend_min_;
    QString legend_max_;
};

#endif // IMAGEVIEWER_H
