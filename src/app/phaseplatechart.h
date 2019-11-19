#ifndef PHASEPLATECHART_H
#define PHASEPLATECHART_H

#include <QGraphicsView>



class PhasePlateChart : public QGraphicsView
{
public:
    PhasePlateChart(QWidget *parent = Q_NULLPTR);
protected:
    QGraphicsScene scene_;
    QList<QGraphicsPathItem*> phase_plates_;
};

#endif // PHASEPLATECHART_H
