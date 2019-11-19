#include "histogram.h"

#include <algorithm>

Histogram::Histogram(float start, float end, int bins):
    start_(std::min<float>(start,end)),
    width_( std::abs(end-start)/std::max<int>(1,bins)),
    bins_(bins,0)
{
    if(width_==0.0f){
        width_=1.0f;
    }
}

void Histogram::add(float value)
{
    int index=static_cast<int>((value-start_)/width_);
    if(index>0 && index< bins_.size()){
        ++bins_[index];
    }
}

void Histogram::add(const QVector<float> &values)
{
    foreach(float value,values){
        add(value);
    }
}

void Histogram::remove(float value)
{
    int index=static_cast<int>((value-start_)/width_);
    if(index>0 && index< bins_.size()){
        --bins_[index];
    }
}

void Histogram::remove(const QVector<float> &values)
{
    foreach(float value,values){
        remove(value);
    }
}

float Histogram::start() const
{
    return start_;
}

float Histogram::end() const
{
    return start_+width_*bins_.size();
}

QVector<int> Histogram::data() const
{
    return bins_;
}

QList<QPointF> Histogram::dataPoints() const
{
    QList<QPointF> datapoints;
    for(int i=0;i<bins_.size();++i){
        datapoints << QPointF(start_+i*width_,bins_[i]);
    }
    return datapoints;
}

float Histogram::width() const
{
    return width_;
}
