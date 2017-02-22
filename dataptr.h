#ifndef DATAPTR_H
#define DATAPTR_H

#include <QHash>
#include <QString>
#include <QSharedPointer>

typedef QHash<QString,QString> Data;
typedef QSharedPointer<Data> DataPtr;

#endif // DATAPTR_H
