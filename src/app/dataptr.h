#ifndef DATAPTR_H
#define DATAPTR_H

#include <QJsonObject>
#include <QString>
#include <QSharedPointer>

//typedef QHash<QString,QString> Data;
typedef QJsonObject Data;
typedef QSharedPointer<Data> DataPtr;

#endif // DATAPTR_H
