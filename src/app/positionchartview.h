#ifndef POSITIONCHARTVIEW_H
#define POSITIONCHARTVIEW_H

#include <QGraphicsView>
#include "positionchart.h"

class PositionChartView : public QGraphicsView
{
    Q_OBJECT
public:
    PositionChartView(QWidget* parent);
    virtual void setScene(QGraphicsScene * s);
signals:
    void clicked(int pos);
protected:
    virtual void resizeEvent(QResizeEvent * event);
    virtual void mousePressEvent(QMouseEvent *event);

private:

};

#endif // POSITIONCHARTVIEW_H
