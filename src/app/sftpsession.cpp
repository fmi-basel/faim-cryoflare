#include "sftpsession.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <QFile>

#define BUFFER_SIZE 16384 //16k

namespace {
    mode_t qt2unixperms(QFileDevice::Permissions permissions){
        return (permissions & 0x7) | ((permissions & 0x70)>>1) | ((permissions & 0x700)>>2) | ((permissions & 0x7000)>>6);
    }
}

SFTPSession::SFTPSession(ssh_session ssh_session):
    session_(sftp_new(ssh_session))
{}

SFTPSession::~SFTPSession()
{
    sftp_free(session_);
}

bool SFTPSession::isValid() const
{
    return session_!=NULL;
}

bool SFTPSession::init()
{
    return sftp_init(session_)==0;
}

int SFTPSession::getError()
{
    if(!isValid()){
        return 0;
    }
    return sftp_get_error(session_);
}

bool SFTPSession::mkDir(const QString &path, QFileDevice::Permissions permissions)
{
    if(!isValid()){
        return false;
    }
    return sftp_mkdir(session_, path.toLatin1().data(), qt2unixperms(permissions))==SSH_OK;
}

SFTPDirList SFTPSession::listDir(const QString &path)
{
    SFTPDirList retval;
    retval.ok=true;

    sftp_dir dir = sftp_opendir(session_, path.toLatin1().data());
    if (dir){
        sftp_attributes attributes;
        while ((attributes = sftp_readdir(session_, dir)) != NULL)
        {
            retval.attribute_list.append(SFTPAttributes{attributes->name,
                attributes->longname,
                attributes->flags,
                attributes->type,
                attributes->size,
                attributes->uid,
                attributes->gid,
                attributes->owner,
                attributes->group,
                attributes->permissions,
                attributes->atime64,
                attributes->atime,
                attributes->atime_nseconds,
                attributes->createtime,
                attributes->createtime_nseconds,
                attributes->mtime64,
                attributes->mtime,
                attributes->mtime_nseconds,
                QString(ssh_string_get_char(attributes->acl)),
                attributes->extended_count,
                QString(ssh_string_get_char(attributes->extended_type)),
                QString(ssh_string_get_char(attributes->extended_data))
            });
            sftp_attributes_free(attributes);
        }
        if (!sftp_dir_eof(dir))
        {
            retval.ok=false;
        }
        if (sftp_closedir(dir) != SSH_OK)
        {
            retval.ok=false;
        }
    } else {
        retval.ok=false;
    }
    return retval;
}

bool SFTPSession::writeFile(std::istream& local, const QString &remote, QFileDevice::Permissions permissions)
{
    int access_type = O_WRONLY | O_CREAT | O_TRUNC;
    std::vector<char> buffer (BUFFER_SIZE,0);

    sftp_file destination_file = sftp_open(session_, remote.toLatin1().data(), access_type, qt2unixperms(permissions));
    if (destination_file == NULL)
    {
        return false;
    }
    while(!local.good()) {
        local.read(buffer.data(), buffer.size());
        std::streamsize s=local.gcount();
        std::streamsize sw = sftp_write(destination_file, buffer.data(), s);
        if (sw != s)
        {
            sftp_close(destination_file);
            return false;
        }
     }
    if (sftp_close(destination_file) != SSH_OK)
    {
         return false;
    }

    return true;
}

bool SFTPSession::writeFile(const QString &local, const QString &remote, QFileDevice::Permissions permissions)
{
    std::ifstream source_stream (local.toLatin1().data(), std::ifstream::binary);
    return writeFile(source_stream,remote,permissions);
}

bool SFTPSession::readFile(const QString &remote, std::ostream &local)
{
    int access_type= O_RDONLY;
    std::vector<char> buffer (BUFFER_SIZE,0);

    sftp_file file = sftp_open(session_, remote.toLatin1().data(), access_type, 0);
    if (file == NULL) {
        return false;
    }
    for (;;) {
        int nbytes = sftp_read(file, buffer.data(), BUFFER_SIZE);
        if (nbytes == 0) {
            break; // EOF
        } else if (nbytes < 0) {
            sftp_close(file);
            return false;
        }

        local.write(buffer.data(), nbytes);
        if (! local.good()) {
            sftp_close(file);
            return false;
        }
    }
    if (sftp_close(file) != SSH_OK) {
         return false;
    }

    return true;
}

bool SFTPSession::readFile(const QString &remote, const QString &local, QFileDevice::Permissions permissions)
{
    std::ofstream destination_stream(local.toLatin1().data(), std::ifstream::binary);
    return readFile(remote, destination_stream);
    destination_stream.close();
    QFile(local).setPermissions(permissions);
}
