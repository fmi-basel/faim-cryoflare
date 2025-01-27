//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2017-2019 by the CryoFLARE Authors
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

#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QStringList>
#include <QDebug>

#include "filelocker.h"

FileLocker::FileLocker(const QString &filename):
    fd_(-1),
    filename_(filename)
{
    if(filename_.isEmpty()){
        qDebug() << "No filename given for FileLocker";
    }
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
    if(filename_.isEmpty()){
        return false;
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
    if(filename_.isEmpty()){
        qDebug() << "No filename given for FileLocker";
        return -1;
    }
    if(!QFileInfo::exists(filename_)){
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
        QStringList sp=line.split(" ",Qt::SkipEmptyParts);
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
