#ifndef GRADIENT_H
#define GRADIENT_H

#include <QColor>
#include <QList>
#include <QMap>

class Gradient
{
public:
    Gradient(const QMap<qreal,QColor> &stops);
    QList<QColor> createColors(const QList<qreal> &values) const;
protected:
    QMap<qreal,QColor> stops_;
};

#endif // GRADIENT_H
