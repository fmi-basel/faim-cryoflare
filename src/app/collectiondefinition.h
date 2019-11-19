#ifndef COLLECTIONDEFINITION_H
#define COLLECTIONDEFINITION_H

#include "dataptr.h"
#include <QVector>
#include <QObject>

class CollectionDefinition : public QObject
{
    Q_OBJECT
public:
    explicit CollectionDefinition(QObject *parent = nullptr);
    QVector<Data> getData();

signals:

public slots:
};

#endif // COLLECTIONDEFINITION_H
