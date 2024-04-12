#ifndef SFTPSESSION_H
#define SFTPSESSION_H

#include <libssh/sftp.h>
#include <QString>
#include <QFileDevice>

struct SFTPAttributes{
    QString     name;
    QString     longname;
    uint32_t 	flags;
    uint8_t 	type;
    uint64_t 	size;
    uint32_t 	uid;
    uint32_t 	gid;
    QString     owner;
    QString     group;
    uint32_t 	permissions;
    uint64_t 	atime64;
    uint32_t 	atime;
    uint32_t 	atime_nseconds;
    uint64_t 	createtime;
    uint32_t 	createtime_nseconds;
    uint64_t 	mtime64;
    uint32_t 	mtime;
    uint32_t 	mtime_nseconds;
    QString 	acl;
    uint32_t 	extended_count;
    QString 	extended_type;
    QString 	extended_data;
};
typedef QList<SFTPAttributes> SFTPAttributeList;
struct SFTPDirList{
    bool ok;
    SFTPAttributeList attribute_list;
};

    class SFTPSession
{
public:
    SFTPSession(ssh_session ssh_session);
    ~SFTPSession();
    bool isValid() const;
    bool init();
    int getError();
    bool mkDir(const QString& path, QFileDevice::Permissions permissions=QFileDevice::ReadUser|QFileDevice::WriteUser|QFileDevice::ExeUser);
    SFTPDirList listDir(const QString& path);
    bool writeFile(std::istream &local, const QString& remote, QFileDevice::Permissions permissions=QFileDevice::ReadUser|QFileDevice::WriteUser);
    bool writeFile(const QString& local, const QString& remote,QFileDevice::Permissions permissions=QFileDevice::ReadUser|QFileDevice::WriteUser);
    bool readFile(const QString& remote, std::ostream &local);
    bool readFile(const QString& remote, const QString& local, QFileDevice::Permissions permissions=QFileDevice::ReadUser|QFileDevice::WriteUser);
private:
    sftp_session session_;
};

#endif // SFTPSESSION_H
