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
#include "sshauthenticationstore.h"

uint qHash(const SshAuthenticationStore::key_type &key){
    return qHash(std::get<0>(key)+std::get<1>(key)+QString("%1").arg(std::get<2>(key)));
}

SshAuthenticationStore::SshAuthenticationStore(QObject *parent) : QObject(parent)
{

}

bool SshAuthenticationStore::contains(const QSsh::SshConnectionParameters &p)
{
    return parameters_.contains(key_type(p.host,p.userName,p.port));
}

const QSsh::SshConnectionParameters SshAuthenticationStore::retrieve(const QSsh::SshConnectionParameters &p)
{
    return parameters_.value(key_type(p.host,p.userName,p.port));
}

void SshAuthenticationStore::store(const QSsh::SshConnectionParameters &p)
{
    parameters_.insert(key_type(p.host,p.userName,p.port),p);
}

void SshAuthenticationStore::storeActiveConnection()
{
    QObject* sender=QObject::sender();
    if(sender){
        QSsh::SshConnection* connection=qobject_cast< QSsh::SshConnection*>(sender);
        if(connection){
            QSsh::SshConnectionParameters p=connection->connectionParameters();
            store(p);
        }
    }
}
QHash<SshAuthenticationStore::key_type,QSsh::SshConnectionParameters > SshAuthenticationStore::parameters_= QHash<SshAuthenticationStore::key_type,QSsh::SshConnectionParameters >();

