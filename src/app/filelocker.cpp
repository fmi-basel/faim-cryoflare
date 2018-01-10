#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>

#include "filelocker.h"

FileLocker::FileLocker(const QString &filename):
    fd_(-1),
    filename_(filename)
{
}

FileLocker::~FileLocker()
{
    release();
}

bool FileLocker::tryLock()
{
    if(fd_>=0){
        return true;
    }
    int fd = open( filename_.toLatin1(), O_RDWR|O_CREAT, 0666 );
    if(fd >= 0 && flock( fd, LOCK_EX | LOCK_NB ) < 0)
    {
        close(fd);
        return false;
    }
    fd_=fd;
    return true;

}

void FileLocker::release()
{
    if(fd_>=0){
        close(fd_);
        fd_=-1;
    }
}

bool FileLocker::isLocked() const
{
    return fd_>=0;
}

int FileLocker::getLockOwner()
{
    if(!QFile(filename_).exists()){
        qDebug() <<filename_<< " doesn't exist";
        return -1;
    }
    struct stat sb;
    if (stat(filename_.toLatin1(), &sb) == -1) {
        qDebug() <<"stat on" <<filename_ << " failed";
        return -1;
    }
    QString file_id=QString("%1:%2:%3").arg(static_cast<long>(major(sb.st_dev)),2,16,QChar('0')).arg(static_cast<long>(minor(sb.st_dev)),0,16,QChar('0')).arg(static_cast<long>(sb.st_ino));
    QFile locks_file("/proc/locks");
    if(!locks_file.open(QIODevice::ReadOnly| QFile::Text)) {
        qDebug() << "cannot open /proc/locks";
        return -1;
    }
    int pid=-1;
    QTextStream in(&locks_file);
    QString line;
    do {
        line = in.readLine();
        QStringList sp=line.split(" ",QString::SkipEmptyParts);
        if(sp.size()<6){
            continue;
        }
        if(sp[5]==file_id){
            pid=sp[4].toInt();
            break;
        }
    } while(!line.isNull());
    locks_file.close();
    return pid;
}
