#ifndef FILESYSTEMWATCHER_H
#define FILESYSTEMWATCHER_H

#include <QObject>
#include <QTimer>
#include <QThread>


//fw decl
class FileSystemWatcherImpl;
class FileSystemWatcher : public QObject
{
    Q_OBJECT
public:
    explicit FileSystemWatcher(QObject *parent = 0);
    FileSystemWatcher(const QStringList & paths, QObject * parent = 0);
    ~FileSystemWatcher();
    bool addPath(const QString & path);
    bool addPaths(const QStringList & paths);
    QStringList directories() const;
    QStringList files() const;
    void removePath(const QString & path);
    void removePaths(const QStringList & paths);

signals:
    void directoryChanged(const QString & path);
    void fileChanged(const QString & path);
public slots:
protected:
    void init_impl();
    QTimer *timer_;
    QThread *thread_;
    FileSystemWatcherImpl* impl_;

};

#endif // FILESYSTEMWATCHER_H
