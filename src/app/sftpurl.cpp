//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2019 by the CryoFLARE Authors
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
