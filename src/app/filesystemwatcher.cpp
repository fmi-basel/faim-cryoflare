#include "filesystemwatcher.h"
#include "filesystemwatcherimpl.h"


FileSystemWatcher::FileSystemWatcher(QObject *parent) :
    QObject(parent),
    timer_(new QTimer),
    thread_(new QThread(this)),
    impl_(new FileSystemWatcherImpl)
{
    init_impl();
}


FileSystemWatcher::FileSystemWatcher(const QStringList &paths, QObject *parent):
    QObject(parent),
    timer_(new QTimer),
    thread_(new QThread(this)),
    impl_(new FileSystemWatcherImpl)
{
    init_impl();
    addPaths(paths);
}

FileSystemWatcher::~FileSystemWatcher()
{

}

void FileSystemWatcher::init_impl()
{
    timer_->setInterval(5000);
    connect(thread_, SIGNAL(started()), timer_, SLOT(start()));
    connect(thread_, SIGNAL(finished()), timer_, SLOT(deleteLater()));
    connect(thread_, SIGNAL(finished()), impl_, SLOT(deleteLater()));
    connect(timer_, SIGNAL(timeout()), impl_, SLOT(update()));

    connect(impl_, SIGNAL(directoryChanged(const QString &)), this, SIGNAL(directoryChanged(const QString &)));
    connect(impl_, SIGNAL(fileChanged(const QString &)), this, SIGNAL(fileChanged(const QString &)));
    timer_->moveToThread(thread_);
    impl_->moveToThread(thread_);
    thread_->start();
}

bool FileSystemWatcher::addPath(const QString &path)
{
    impl_->addPath(path);
    return true;
}

bool FileSystemWatcher::addPaths(const QStringList &paths)
{
    impl_->addPaths(paths);
    return true;
}

QStringList FileSystemWatcher::directories() const
{
    return impl_->directories();
}

QStringList FileSystemWatcher::files() const
{
    return impl_->files();
}

void FileSystemWatcher::removePath(const QString &path)
{
    impl_->removePath(path);
}

void FileSystemWatcher::removePaths(const QStringList &paths)
{
    impl_->removePaths(paths);
}

