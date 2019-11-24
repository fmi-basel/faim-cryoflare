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

#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QChartView>
#include <QRubberBand>

//fw decl
class QPushButton;
class ImageTableModel;

class ChartView : public QtCharts::QChartView
{
    Q_OBJECT
public:
    ChartView(QWidget *parent = Q_NULLPTR);
    void setModel(ImageTableModel *model);
    void setActiveColumn(int column);
    QColor color() const;
    void setColor(const QColor &color);

    QColor selectedColor() const;
    void setSelectedColor(const QColor &selectedColor);

public slots:
    void enableSelection(bool selecting);
    void update();
protected:
    void drawChart_();
    virtual void drawSeries_();
    virtual void deselectData_(float start, float end, bool invert=false){}
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    QPointF mapToChartValue_(const QPoint & p) const;
    ImageTableModel *model_;
    int active_column_;
    QPushButton *fit_button_;
    QColor color_;
    QColor selected_color_;
private:
    bool selecting_;
    QRubberBand *rubberband_;
    QPoint rubberband_start_;
    QTimer *timer_;
};
#endif // CHARTVIEW_H
