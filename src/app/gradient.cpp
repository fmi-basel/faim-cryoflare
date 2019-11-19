#include <algorithm>
#include "gradient.h"

Gradient::Gradient(const QMap<qreal,QColor> &stops):
    stops_(stops)
{
}

QList<QColor> Gradient::createColors(const QList<qreal>& values) const
{
    QList<QColor> result;
    QList<qreal> positions=stops_.keys();
    foreach(qreal value, values){
        if(qIsNaN(value)){
            result.append(QColor::fromRgbF(1.0,1.0,1.0,1.0));
            continue;
        }
        QList<qreal>::iterator lower_bound=std::lower_bound (positions.begin(), positions.end(), value);
        if(lower_bound==positions.begin()){
            result.append(stops_.first());
            continue;
        }
        if(lower_bound==positions.end()){
            result.append(stops_.last());
            continue;
        }
        qreal upper_pos=*lower_bound;
        --lower_bound;
        qreal lower_pos=*lower_bound;
        QColor lower_color=stops_.value(lower_pos);
        QColor upper_color=stops_.value(upper_pos);
        qreal interval=upper_pos-lower_pos;
        qreal rel_offset=(value-lower_pos)/interval;
        qreal irel_offset=1.0-rel_offset;
        qreal red=std::max(0.0,std::min(1.0,lower_color.redF()*irel_offset+upper_color.redF()*rel_offset));
        qreal green=std::max(0.0,std::min(1.0,lower_color.greenF()*irel_offset+upper_color.greenF()*rel_offset));
        qreal blue=std::max(0.0,std::min(1.0,lower_color.blueF()*irel_offset+upper_color.blueF()*rel_offset));
        qreal alpha=std::max(0.0,std::min(1.0,lower_color.alphaF()*irel_offset+upper_color.alphaF()*rel_offset));
        result.append(QColor::fromRgbF(red,green,blue,alpha));
    }
    return result;
}
