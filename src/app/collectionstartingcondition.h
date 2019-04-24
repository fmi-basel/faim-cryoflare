#ifndef COLLECTIONSTARTINGCONDITION_H
#define COLLECTIONSTARTINGCONDITION_H

#include <QObject>

class CollectionStartingCondition : public QObject
{
    Q_OBJECT
public:
    explicit CollectionStartingCondition(QObject *parent = nullptr);
    bool fullfilled() const;

signals:

public slots:
};

#endif // COLLECTIONSTARTINGCONDITION_H
