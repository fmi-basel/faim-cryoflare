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
