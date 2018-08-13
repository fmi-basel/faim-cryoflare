#include "sftpurl.h"

SftpUrl::SftpUrl():
    QUrl(),
    key_(),
    auth_type_()
{

}

SftpUrl::SftpUrl(const QSsh::SshConnectionParameters& p):
    QUrl(),
    key_(),
    auth_type_(p.authenticationType)
{
    setHost(p.host);
    setUserName(p.userName);
    setPort(p.port);
    if(QSsh::SshConnectionParameters::AuthenticationByPassword==p.authenticationType){
        setPassword(p.password);
    }else{
        setKey(p.privateKeyFile);
    }
}

SftpUrl::SftpUrl(const QUrl &other):
    QUrl(other),
    key_(),
    auth_type_()
{
    if(other.scheme()=="sftp"){
        auth_type_=QSsh::SshConnectionParameters::AuthenticationByPassword;
    }
}

SftpUrl::SftpUrl(const QString &url, QUrl::ParsingMode parsingMode):
    QUrl(url,parsingMode),
    key_(),
    auth_type_()
{
    if(scheme()=="sftp"){
        auth_type_=QSsh::SshConnectionParameters::AuthenticationByPassword;
    }
}

SftpUrl::SftpUrl(QUrl &&other):
    QUrl(other),
    key_(),
    auth_type_()
{
    if(scheme()=="sftp"){
        auth_type_=QSsh::SshConnectionParameters::AuthenticationByPassword;
    }
}

QSsh::SshConnectionParameters SftpUrl::toConnectionParameters() const
{
    QSsh::SshConnectionParameters result;
    result.host=host();
    result.userName=userName();
    result.port=port();
    result.authenticationType=authType();
    result.timeout=30;
    result.proxyType=QSsh::SshConnectionParameters::NoProxy;
    if(authType()==QSsh::SshConnectionParameters::AuthenticationByPassword){
        result.password=password();
    }else{
        result.privateKeyFile=key();
    }
    return result;
}

QString SftpUrl::key() const
{
    return key_;
}

void SftpUrl::setKey(const QString &key)
{
    key_ = key;
}

QSsh::SshConnectionParameters::AuthenticationType SftpUrl::authType() const
{
    return auth_type_;
}

void SftpUrl::setAuthType(const QSsh::SshConnectionParameters::AuthenticationType &auth_type)
{
    auth_type_ = auth_type;
}
