//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2020 by the CryoFLARE Authors
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3.0 of the License.
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License
// along with CryoFLARE.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------------
#ifndef DATAPTR_H
#define DATAPTR_H

#include <QJsonObject>
#include <QString>
#include <QSet>
#include <QSharedPointer>
#include <QDateTime>
#include <QJsonArray>

const QString timeformat("yyyy-MM-dd hh:mm:ss.z");

class Data: public QJsonObject{
public:
    Data(const QJsonObject& other):
        QJsonObject(other)
    {}
    Data():
        QJsonObject()
    {}
    QDateTime timestamp() const{
        if(! contains("last_modified")){
            return QDateTime();
        }
        return QDateTime::fromString(value("last_modified").toString(),timeformat);
    }
    void setTimestamp(const QDateTime& dt){
        insert("last_modified",dt.toString(timeformat));
    }
    QString id() const
    {
        if(contains("id")){
            return value("id").toString();
        }
        return "";
    }
    void setId(const QString& id){
        insert("id",id);
    }
    QString parent() const{
        {
            if(contains("parent")){
                return value("parent").toString();
            }
            return "";
        }
    }
    void setParent(const QString& id) {
        insert("parent",id);
    }
    QSet<QString> children() const{
        QSet<QString> result;
        if(contains("children")){
            foreach(QJsonValue val, value("children").toArray())
            result << val.toString();
        }
        return result;
    }
    void addChild(const QString & child){
        QJsonArray array;
        if(contains("children")){
            array= value("children").toArray();
        }
        if(! array.contains(child)){
            array.append(child);
            insert("children",array);
        }
    }
    void update(const Data& other){
        QDateTime other_timestamp=other.timestamp();
        QDateTime this_timestamp=timestamp();
        foreach(QString key, other.keys()){
            if(key!="children" && (other_timestamp>this_timestamp || ! contains(key))){
                insert(key,other.value(key));
            }
        }
        foreach( QString child, other.children()){
            addChild(child);
        }
    }
};
typedef QSharedPointer<Data> DataPtr;

#endif // DATAPTR_H
