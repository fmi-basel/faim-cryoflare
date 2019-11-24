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
#ifndef SSHURL_H
#define SSHURL_H

#include <QUrl>
#include "../external/qssh/sshconnection.h"

class SftpUrl : public QUrl
{
public:
    SftpUrl();
    SftpUrl(const QSsh::SshConnectionParameters& p);
    SftpUrl(const QUrl &other);
    SftpUrl(const QString &url, QUrl::ParsingMode parsingMode = TolerantMode);
    QSsh::SshConnectionParameters toConnectionParameters() const;
    QString key() const;
    void setKey(const QString &key);

    QSsh::SshConnectionParameters::AuthenticationType authType() const;
    void setAuthType(const QSsh::SshConnectionParameters::AuthenticationType &auth_type);

protected:
    QString key_;
    QSsh::SshConnectionParameters::AuthenticationType auth_type_;
};

#endif // SSHURL_H
