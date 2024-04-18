#include "sftpsession.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include <QFile>
#include <QDataStream>

#define BUFFER_SIZE 16384 //16k

namespace {
    mode_t qt2unixperms(QFileDevice::Permissions permissions){
        return (permissions & 0x7) | ((permissions & 0x70)>>1) | ((permissions & 0x700)>>2) | ((permissions & 0x7000)>>6);
    }

    SFTPAttributes attributes2qtattributes(sftp_attributes attributes)
        {
            return SFTPAttributes{attributes->name,
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
            };
        }

}


SFTPSession::SFTPSession():
    ssh_session_(),
    sftp_session_()
{
}

SFTPSession::~SFTPSession()
{
}

sftp_session SFTPSession::session() const
{
    return sftp_session_.data();

}

bool SFTPSession::isValid() const
{
    return session()!=NULL;
}


bool SFTPSession::connect(const QUrl &url)
{
    ssh_session_=SSHSession::createAuthenticatedSession(url);
    if(ssh_session_.isConnected()){
        sftp_session_=QSharedPointer<struct sftp_session_struct>(sftp_new(ssh_session_.session()),sftp_free);
        return sftp_init(session())==0;
    }
    return false;
}

void SFTPSession::disconnect()
{
    sftp_session_.clear();
    ssh_session_=SSHSession();
}

QUrl SFTPSession::getUrl() const
{
    return ssh_session_.getUrl();
}

int SFTPSession::getError()
{
    if(!isValid()){
        return 0;
    }
    return sftp_get_error(session());
}

bool SFTPSession::mkDir(const QString &path, QFileDevice::Permissions permissions)
{
    if(!isValid()){
        return false;
    }
    return sftp_mkdir(session(), path.toLatin1().data(), qt2unixperms(permissions))==SSH_OK;
}

SFTPDirList SFTPSession::listDir(const QString &path)
{
    SFTPDirList retval;
    retval.ok=true;

    sftp_dir dir = sftp_opendir(session(), path.toLatin1().data());
    if (dir){
        sftp_attributes attributes;
        while ((attributes = sftp_readdir(session(), dir)) != NULL)
        {
            retval.attribute_list.append(attributes2qtattributes(attributes));
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

bool SFTPSession::writeFile(QDataStream &local, const QString &remote, QFileDevice::Permissions permissions)
{
    int access_type = O_WRONLY | O_CREAT | O_TRUNC;
    std::vector<char> buffer (BUFFER_SIZE,0);

    sftp_file destination_file = sftp_open(session(), remote.toLatin1().data(), access_type, qt2unixperms(permissions));
    if (destination_file == NULL)
    {
        return false;
    }
    while(QDataStream::Ok == local.status()) {
        int s=local.readRawData(buffer.data(), buffer.size());
        if (s < 0)
        {
            sftp_close(destination_file);
            return false;
        }
        int sw = sftp_write(destination_file, buffer.data(), s);
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
    QFile file(local);
    file.open(QIODevice::ReadOnly);
    QDataStream source_stream(&file);
    return writeFile(source_stream,remote,permissions);
}

bool SFTPSession::readFile(const QString &remote, QDataStream &local)
{
    int access_type= O_RDONLY;
    std::vector<char> buffer (BUFFER_SIZE,0);

    sftp_file file = sftp_open(session(), remote.toLatin1().data(), access_type, 0);
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

        int s=local.writeRawData(buffer.data(), nbytes);
        if ( s< 0 || s != nbytes || QDataStream::Ok != local.status()) {
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
    QFile file(local);
    file.open(QIODevice::WriteOnly);
    QDataStream destination_stream(&file);
    bool retval = readFile(remote, destination_stream);
    file.close();
    file.setPermissions(permissions);
    return retval;
}

bool SFTPSession::createLink(const QString &target, const QString &destination)
{
    return sftp_symlink(session(), target.toLatin1().data(), destination.toLatin1().data() )==0;
}

SFTPAttributes SFTPSession::stat(const QString &remote)
{
    sftp_file remote_file = sftp_open(session(), remote.toLatin1().data(), O_RDONLY, 0);
    if(remote_file == NULL){
        return SFTPAttributes();
    }
    sftp_attributes attributes=sftp_stat(session(),remote.toLatin1().data());
    SFTPAttributes retval=attributes2qtattributes(attributes);
    sftp_attributes_free(attributes);
    return retval;
}

bool SFTPSession::exists(const QString &remote)
{
    sftp_file remote_file = sftp_open(session(), remote.toLatin1().data(), O_RDONLY, 0);
    if(remote_file != NULL){
        sftp_close(remote_file);
        return true;
    }else{
        return SSH_FX_PERMISSION_DENIED == getError();
    }
}

bool SFTPSession::isDir(const QString &remote)
{
    sftp_dir dir = sftp_opendir(session(), remote.toLatin1().data());
    if(dir != NULL){
        sftp_closedir(dir);
        return true;
    }
    return SSH_FX_PERMISSION_DENIED == getError();
}
