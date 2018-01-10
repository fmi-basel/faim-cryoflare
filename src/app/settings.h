#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QSettings>

//fw decl
class SettingsGroup;

class Settings: public QObject
{
    Q_OBJECT
public:
    Settings(QObject *parent = NULL);
    bool loadFromFile(const QString& path, const QStringList &excludes=QStringList(), const QStringList &includes=QStringList());
    void saveToFile(const QString& path, const QStringList &excludes=QStringList(), const QStringList &includes=QStringList()) const;
    void loadFromQSettings(const QStringList &excludes=QStringList(), const QStringList &includes=QStringList());
    void saveToQSettings(const QStringList &excludes=QStringList(), const QStringList &includes=QStringList()) const;
    void setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;
    bool contains(const QString &key);
    void beginGroup(const QString &prefix);
    void endGroup();
    QStringList childGroups() const;
    QStringList allKeys() const;
    QStringList childKeys() const;
    void remove(const QString & key);
    void clear();
protected:
    SettingsGroup* current_;
};
#endif // SETTINGS_H
