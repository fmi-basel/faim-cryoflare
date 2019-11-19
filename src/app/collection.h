#ifndef COLLECTION_H
#define COLLECTION_H

#include "dataptr.h"
#include <QObject>
#include <QVector>

//fw decl
class CollectionStartingCondition;
class CollectionDefinition;

class Collection : public QObject
{
    Q_OBJECT
public:
    explicit Collection(QObject *parent = nullptr);

signals:

public slots:
    void checkStartingCondition();
    void start();
protected:
    void assembleData_();
    void submitJob();
    CollectionStartingCondition* starting_condition_;
    CollectionDefinition* definition_;
    QVector<Data> data_;
};

#endif // COLLECTION_H
