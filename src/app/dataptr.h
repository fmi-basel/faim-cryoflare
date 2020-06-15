#ifndef DATAPTR_H
#define DATAPTR_H

#include <QJsonObject>
#include <QString>
#include <QSet>
#include <QSharedPointer>
#include <QDateTime>

const QString timeformat("yyyy-MM-dd hh:mm:ss.z");

class Data: public QJsonObject{
public:
    Data(const QJsonObject& other):
        QJsonObject(other),
        parent(),
        children()
    {}
    Data():
        QJsonObject(),
        parent(),
        children()
    {}
    QDateTime timestamp(){
        if(! contains("last_modified")){
            return QDateTime();
        }
        return QDateTime::fromString(value("last_modified").toString(),timeformat);
    }
    void setTimestamp(const QDateTime& dt){
        insert("last_modified",dt.toString(timeformat));
    }
    QString parent;
    QSet<QString> children;
};
typedef QSharedPointer<Data> DataPtr;

#endif // DATAPTR_H
