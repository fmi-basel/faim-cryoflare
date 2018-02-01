#include "chartview.h"

ChartView::ChartView(QWidget *parent):
    QtCharts::QChartView(parent),
    selecting_(false),
    rubberband_(new QRubberBand(QRubberBand::Rectangle,this))
{

}

void ChartView::enableSelection(bool selecting)
{
    selecting_=selecting;
}

void ChartView::mousePressEvent(QMouseEvent *event)
{
    if(selecting_){
        QRect plot_area =chart()->plotArea().toRect();
        rubberband_start_=event->pos();
        rubberband_start_.setY(plot_area.top());
        rubberband_->setGeometry(QRect(rubberband_start_,QSize()));
        rubberband_->show();
    }
}

void ChartView::mouseMoveEvent(QMouseEvent *event)
{
    if(selecting_){
        QRect plot_area =chart()->plotArea().toRect();
        int width = event->pos().x()-rubberband_start_.x();
        rubberband_->setGeometry(QRect(rubberband_start_,QSize(width,plot_area.height())).normalized());
    }
}


void ChartView::mouseReleaseEvent(QMouseEvent *event)
{
    if(selecting_){
        rubberband_->hide();
        QRect selection=rubberband_->geometry();
        selecting_=false;
        emit selected(mapToChartValue_(selection.topLeft()).x(),mapToChartValue_(selection.bottomRight()).x(),event->modifiers() & Qt::ShiftModifier);
    }
}

QPointF ChartView::mapToChartValue_(const QPoint &p) const
{
    return chart()->mapToValue(chart()->mapFromScene(mapToScene(p)));
}
