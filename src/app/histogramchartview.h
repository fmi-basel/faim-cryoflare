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
