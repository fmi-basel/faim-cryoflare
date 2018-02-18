#include <QtDebug>
#include <QMouseEvent>
#include <QGraphicsItem>
#include "positionchartview.h"
#include <algorithm>

PositionChartView::PositionChartView(QWidget *parent):
    QGraphicsView(parent)
{
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);
}

void PositionChartView::setScene(QGraphicsScene *s)
{
    QGraphicsView::setScene(s);
    fitInView(scene()->sceneRect(),Qt::KeepAspectRatio );
}

void PositionChartView::resizeEvent(QResizeEvent *event)
{
    if(! scene()){
        return;
    }
    fitInView(scene()->sceneRect(),Qt::KeepAspectRatio );
}

void PositionChartView::mousePressEvent(QMouseEvent *event)
{
    if (QGraphicsItem *item = itemAt(event->pos())) {
        if(item->toolTip()!=""){
            emit clicked(item->toolTip().toInt());
        }
     }
}
