#include "collectionstartingcondition.h"

CollectionStartingCondition::CollectionStartingCondition(QObject *parent) : QObject(parent)
{

}

bool CollectionStartingCondition::fullfilled() const
{
    return true;
}
