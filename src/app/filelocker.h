#ifndef FILELOCKER_H
#define FILELOCKER_H

#include <QString>

class FileLocker
{
public:
    FileLocker(const QString& filename);
    ~FileLocker();
    bool tryLock();
    void release();
    bool isLocked() const;
    int getLockOwner();
protected:
    int fd_;
    QString filename_;
};

#endif // FILELOCKER_H
