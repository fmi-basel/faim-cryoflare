#ifndef LINEARCHARTVIEW_H
#define LINEARCHARTVIEW_H

#include "chartview.h"


class LinearChartView : public ChartView
{
public:
    LinearChartView(QWidget *parent = Q_NULLPTR);

protected:
    virtual void drawSeries_();
    virtual void deselectData_(float start, float end, bool invert=false);
};

#endif // LINEARCHARTVIEW_H
