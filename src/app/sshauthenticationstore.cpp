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

