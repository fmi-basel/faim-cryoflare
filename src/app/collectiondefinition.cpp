#include "collectiondefinition.h"

CollectionDefinition::CollectionDefinition(QObject *parent) : QObject(parent)
{

}

QVector<DataPtr> CollectionDefinition::getData()
{
    return QVector<DataPtr>();
}
