#ifndef SSHSESSION_H
#define SSHSESSION_H

#include <libssh/libssh.h>
#include <QList>
#include <QUrl>

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
    SSHSession(const QString& host="Localhost", int port=22);
    SSHSession(const QUrl& url);
    ~SSHSession();
    static SSHSession createAuthenticatedSession(const QUrl& url);
    static QUrl createAuthenticatedUrl(const QUrl& url);
    bool isValid() const;
    bool connect();
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
protected:
    ssh_session session_;

};

#endif // SSHSESSION_H
