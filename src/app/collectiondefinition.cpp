#include "collectiondefinition.h"

CollectionDefinition::CollectionDefinition(QObject *parent) : QObject(parent)
{

}

QVector<Data> CollectionDefinition::getData()
{
    return QVector<Data>();
}
