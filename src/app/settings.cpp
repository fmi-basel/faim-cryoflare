#include "settings.h"
#include <QHash>
#include <QList>
#include <QQueue>
#include <QPair>
#include <QFile>
#include <QCoreApplication>
#include <QStringList>


class SettingsGroup: public QObject
{
public:
    explicit SettingsGroup():
        QObject(QCoreApplication::instance()),
        parent_group(NULL),
        name(),
        child_groups(),
        values()
    {}
    SettingsGroup(SettingsGroup* parent, const QString& name_):
        QObject(parent),
        parent_group(parent),
        name(name_),
        child_groups(),
        values()
    {}
    ~SettingsGroup(){
    }

    SettingsGroup* childGroup(const QString& name,bool create=false){
        foreach(SettingsGroup* child,child_groups){
            if(child->name==name){
                return child;
            }
        }
        if(!create){
            return NULL;
        }
        SettingsGroup* new_child=new SettingsGroup(this,name);
        child_groups.append(new_child);
        return new_child;
    }

    SettingsGroup* parent_group;
    QString name;
    QList<SettingsGroup*> child_groups;
    QHash<QString,QVariant> values;
};

Settings::Settings(QObject *parent):
    QObject(parent),
    current_()
{
    static SettingsGroup* root;
    if(!root){
        root=new SettingsGroup();
    }
    current_=root;
}

bool Settings::loadFromFile(const QString &path, const QStringList &excludes, const QStringList &includes)
{
    if(!QFile(path).exists()){
        return false;
    }
    foreach( QString key, allKeys()){
        if(!excludes.contains(key) && (includes.empty() || includes.contains(key))){
            remove(key);
        }
    }
    QSettings settings(path,QSettings::IniFormat);
    foreach( QString key, settings.allKeys()){
        if(!excludes.contains(key) && (includes.empty() || includes.contains(key))){
            setValue(key,settings.value(key));
        }
    }
    return true;
}

void Settings::saveToFile(const QString &path, const QStringList &excludes, const QStringList &includes) const
{
    QSettings settings(path,QSettings::IniFormat);
    foreach( QString key, settings.allKeys()){
        if(!excludes.contains(key) && (includes.empty() || includes.contains(key))){
            settings.remove(key);
        }
    }
    foreach( QString key, allKeys()){
        if(!excludes.contains(key) && (includes.empty() || includes.contains(key))){
            settings.setValue(key,value(key));
        }
    }
}

void Settings::loadFromQSettings(const QStringList &excludes, const QStringList &includes)
{
    QSettings settings;
    foreach( QString key, allKeys()){
        if(!excludes.contains(key) && (includes.empty() || includes.contains(key))){
            remove(key);
        }
    }
    foreach( QString key, settings.allKeys()){
        if(!excludes.contains(key) && (includes.empty() || includes.contains(key))){
            setValue(key,settings.value(key));
        }
    }
}

void Settings::saveToQSettings(const QStringList &excludes, const QStringList &includes) const
{
    QSettings settings;
    foreach( QString key, settings.allKeys()){
        if(!excludes.contains(key) && (includes.empty() || includes.contains(key))){
            settings.remove(key);
        }
    }
    foreach( QString key, allKeys()){
        if(!excludes.contains(key) && (includes.empty() || includes.contains(key))){
            settings.setValue(key,value(key));
        }
    }
}

void Settings::setValue(const QString &key, const QVariant &value)
{
    QStringList keylist=key.split("/");
    SettingsGroup* group=current_;
    while(keylist.size()>1){
        QString group_name=keylist.takeFirst();
        group=group->childGroup(group_name,true);
    }
    group->values.insert(keylist.takeFirst(),value);

}

QVariant Settings::value(const QString &key, const QVariant &defaultValue) const
{
    QStringList keylist=key.split("/");
    SettingsGroup* group=current_;
    while(keylist.size()>1){
        QString group_name=keylist.takeFirst();
        if(! (group=group->childGroup(group_name))){
            return defaultValue;
        }
    }
    return group->values.value(keylist.takeFirst(),defaultValue);
}

bool Settings::contains(const QString &key)
{
    QStringList keylist=key.split("/");
    SettingsGroup* group=current_;
    while(keylist.size()>1){
        QString group_name=keylist.takeFirst();
        if(! (group=group->childGroup(group_name))){
            return false;
        }
    }
    return true;

}

void Settings::beginGroup(const QString &prefix)
{
    current_=current_->childGroup(prefix,true);
}

void Settings::endGroup()
{
    if(current_->parent_group){
        current_=current_->parent_group;
    }
}

QStringList Settings::childGroups() const
{
    QStringList names;
    foreach(SettingsGroup* group,current_->child_groups){
        names << group->name;
    }
    return names;
}

QStringList Settings::allKeys() const
{
    QStringList result;
    typedef QPair<QString,SettingsGroup*> GroupPair;
    QQueue<GroupPair> queue;
    queue.enqueue(GroupPair("",current_));
    while(!queue.empty()){
        GroupPair pair=queue.dequeue();
        foreach(QString key_name,pair.second->values.keys()){
            result << pair.first+key_name;
        }
        foreach(SettingsGroup* child_group,pair.second->child_groups){
            queue.enqueue(GroupPair(pair.first+child_group->name+"/",child_group));
        }
    }
    return result;
}

QStringList Settings::childKeys() const
{
    return current_->values.keys();
}

void Settings::remove(const QString &key)
{
    if(key==QString("")){
        current_->values.clear();
        while(! current_->child_groups.empty()){
            current_->child_groups.takeLast()->deleteLater();
        }
    }else{
        current_->values.remove(key);
        for(int i=0;i<current_->child_groups.size();++i){
            if(current_->child_groups[i]->name==key){
                current_->child_groups.takeAt(i)->deleteLater();
                break;
            }
        }
    }
}

void Settings::clear()
{
    while (current_->parent_group) {
        endGroup();
    }
    remove("");
}
