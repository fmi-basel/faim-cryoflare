#ifndef SFTPSESSION_H
#define SFTPSESSION_H

#include <libssh/sftp.h>
#include "sshsession.h"
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
    SFTPSession();
    ~SFTPSession();
    sftp_session session() const;
    bool isValid() const;
    bool connect(const QUrl& url);
    void disconnect();
    QUrl getUrl() const;
    int getError() const;
    QString getErrorString() const;
    bool mkDir(const QString& path, QFileDevice::Permissions permissions);
    SFTPDirList listDir(const QString& path);
    bool writeFile(QDataStream &local, const QString& remote, QFileDevice::Permissions permissions);
    bool writeFile(const QString& local, const QString& remote,QFileDevice::Permissions permissions);
    bool readFile(const QString& remote, QDataStream &local);
    bool readFile(const QString& remote, const QString& local, QFileDevice::Permissions permissions);
    bool createLink(const QString& target, const QString& destination);
    SFTPAttributes stat(const QString& remote);
    bool exists(const QString& remote);
    bool isDir(const QString& remote);
 private:
    SSHSession ssh_session_;
    QSharedPointer<struct sftp_session_struct> sftp_session_;
    QUrl url_;
};

#endif // SFTPSESSION_H
