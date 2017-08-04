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
    Settings(QObject *parent = Q_NULLPTR);
    bool loadFromFile(const QString& path);
    void saveToFile(const QString& path) const;
    void loadFromQSettings();
    void saveToQSettings() const;
    void setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;
    void beginGroup(const QString &prefix);
    void endGroup();
    QStringList childGroups() const;
    QStringList allKeys() const;
    QStringList childKeys() const;
    void remove(const QString & key);
protected:
    SettingsGroup* current_;
};
#endif // SETTINGS_H
