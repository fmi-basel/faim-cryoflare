#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QChartView>
#include <QRubberBand>

class ChartView : public QtCharts::QChartView
{
    Q_OBJECT
public:
    ChartView(QWidget *parent = Q_NULLPTR);
public slots:
    void enableSelection(bool selecting);
signals:
    void selected(float start, float end, bool invert=false);
protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    QPointF mapToChartValue_(const QPoint & p) const;
private:
    bool selecting_;
    QRubberBand *rubberband_;
    QPoint rubberband_start_;
};
#endif // CHARTVIEW_H
