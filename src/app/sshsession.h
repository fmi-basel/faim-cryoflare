#ifndef SSHSESSION_H
#define SSHSESSION_H

#include <libssh/libssh.h>
#include <QList>
#include <QUrl>
#include <QSharedPointer>

struct SSHHostVerification{
    int status;
    QString hexa;
};

struct SSHKbdIntPrompt{
    QString prompt;
    bool echo;
};
struct SSHKbdIntPromptList{
    QString name;
    QString instructions;
    QList<SSHKbdIntPrompt> prompts;
};

class SSHSession
{
public:
    SSHSession();
    ~SSHSession();
    static SSHSession createAuthenticatedSession(const QUrl& url);
    bool isValid() const;
    bool connect(const QUrl& url);
    QUrl getUrl() const;
    void disconnect();
    bool isConnected() const;
    QString getError();
    SSHHostVerification authenticateHost();
    bool updateKnownHosts();
    int getUserAuthList();
    int authenticatePubKey();
    int authenticatePassword(const QString password);
    int authenticateKbInt();
    SSHKbdIntPromptList getKbdIntPrompts();
    int setKbdIntAnswers(const QStringList& answers);
    ssh_session session() const;
protected:
    QSharedPointer<struct ssh_session_struct> session_;
    QUrl url_;

};

#endif // SSHSESSION_H
