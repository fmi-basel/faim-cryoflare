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
    SftpUrl(QUrl &&other);
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
