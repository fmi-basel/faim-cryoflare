#include "settings.h"

#include <QFile>

Settings::Settings(QObject *parent):
    QObject(parent),
    groups_()
{

}

bool Settings::loadFromFile(const QString &path)
{
    if(!QFile(path).exists()){
        return false;
    }
    QSettings settings(path);
    foreach( QString key, settings.allKeys()){
        values_.insert(key,values_.value(key));
    }
    return true;
}

void Settings::saveToFile(const QString &path) const
{
    QSettings settings(path);
    foreach( QString key, values_.keys()){
        settings.setValue(key,values_.value(key));
    }
}

void Settings::loadFromQSettings()
{
    QSettings settings;
    foreach( QString key, settings.allKeys()){
        values_.insert(key,values_.value(key));
    }
}

void Settings::saveToQSettings() const
{
    QSettings settings;
    foreach( QString key, values_.keys()){
        settings.setValue(key,values_.value(key));
    }
}

void Settings::setValue(const QString &key, const QVariant &value) const
{
    if(groups_.empty()){
        values_.insert(key,value);
    }else{
        values_.insert(groups_.join("/")+"/"+key,value);
    }
}

QVariant Settings::value(const QString &key, const QVariant &defaultValue)
{
    if(groups_.empty()){
        return values_.value(key,defaultValue);
    }else{
        return values_.value(groups_.join("/")+"/"+key,defaultValue);
    }
}

void Settings::beginGroup(const QString &prefix)
{
    groups_.append(prefix);
}

void Settings::endGroup()
{
    if(!groups_.empty()){
        groups_.removeLast();
    }
}

QHash<QString,QVariant> Settings::values_=QHash<QString,QVariant>();
