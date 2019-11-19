#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <QPointF>
#include <QVector>



class Histogram
{
public:
    Histogram(float start, float end, int bins);
    void add(float value);
    void add(const QVector<float>& values);
    void remove(float value);
    void remove(const QVector<float>& values);
    float start() const;
    float end() const;
    QVector<int> data() const;
    QList<QPointF> dataPoints() const;
    float width() const;

protected:
    float start_;
    float width_;
    QVector<int> bins_;
};

#endif // HISTOGRAM_H
