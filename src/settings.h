#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QVariant>
#include <QSettings>


class Settings: public QObject
{
    Q_OBJECT
public:
    Settings(QObject *parent = Q_NULLPTR);
    bool loadFromFile(const QString& path);
    void saveToFile(const QString& path) const;
    void loadFromQSettings();
    void saveToQSettings() const;
    void setValue(const QString &key, const QVariant &value) const;
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant());
    void beginGroup(const QString &prefix);
    void endGroup();
protected:
    QStringList groups_;
    static QHash<QString,QVariant> values_;
};
#endif // SETTINGS_H
