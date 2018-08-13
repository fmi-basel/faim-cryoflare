#ifndef SSHAUTHENTICATIONSTORE_H
#define SSHAUTHENTICATIONSTORE_H

#include <QHash>
#include <QObject>
#include"../external/qssh/sshconnection.h"

class SshAuthenticationStore : public QObject
{
    Q_OBJECT
public:
    typedef std::tuple<QString,QString,quint16> key_type;
    explicit SshAuthenticationStore(QObject *parent = nullptr);
    bool contains(const QSsh::SshConnectionParameters& p);
    const QSsh::SshConnectionParameters retrieve(const QSsh::SshConnectionParameters& p);
signals:

public slots:
    void store(const QSsh::SshConnectionParameters& p);
    void storeActiveConnection();
protected:
    static QHash<key_type,QSsh::SshConnectionParameters > parameters_;
};

#endif // SSHAUTHENTICATIONSTORE_H
