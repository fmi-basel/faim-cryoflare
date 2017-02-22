#ifndef FILESYSTEMWATCHERIMPL_H
#define FILESYSTEMWATCHERIMPL_H

#include <QObject>
#include <QHash>
#include <QDir>
#include <QMutex>
#include <QDateTime>

class FileSystemWatcherImpl : public QObject
{
    Q_OBJECT
public:
    explicit FileSystemWatcherImpl(QObject *parent = 0);

    void addPath(const QString & path);

    void addPaths(const QStringList & paths);

    QStringList directories() const;

    QStringList files() const;

    void removePath(const QString & path);

    void removePaths(const QStringList & paths);
signals:
    void directoryChanged(const QString & path);
    void fileChanged(const QString & path);

public slots:
    void update();

protected:
    QStringList files_;
    QStringList dirs_;
    QHash<QString,QDateTime> file_mod_times_;
    QHash<QString,QHash<QString,QDateTime> > dir_file_mod_times_;
    mutable QMutex mutex;
};

#endif // FILESYSTEMWATCHERIMPL_H
