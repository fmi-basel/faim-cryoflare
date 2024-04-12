#include "sshsession.h"
#include <QInputDialog>
#include <QObject>
#include <QMessageBox>

namespace {

struct auth_result{
    SSHSession session;
    QUrl url;
};

auth_result authenticate_(const QUrl &url){
    SSHSession session(url);
    if(!session.connect()){
        QMessageBox::critical(nullptr,QObject::tr("SSH error during connection"),session.getError());
        return auth_result();
    }
    SSHHostVerification host_ver=session.authenticateHost();
    switch(host_ver.status){
    case SSH_KNOWN_HOSTS_ERROR:
        QMessageBox::critical(nullptr,QObject::tr("SSH host verification error"),session.getError());
        return auth_result();
    case SSH_KNOWN_HOSTS_OK:
        break;
    case SSH_KNOWN_HOSTS_CHANGED:
        QMessageBox::critical(nullptr,QObject::tr("SSH host verification changed"),QObject::tr("Host key for server changed: it is now:\n%1\nFor security reasons, connection will be stopped").arg(host_ver.hexa));
        return auth_result();
    case SSH_KNOWN_HOSTS_OTHER:
        QMessageBox::critical(nullptr,QObject::tr("SSH host verification changed"),QObject::tr("The host key for this server was not found but an other type of key exists.\nAn attacker might change the default server key to confuse your client into thinking the key does not exist.\nFor security reasons, connection will be stopped."));
        return auth_result();
    case SSH_KNOWN_HOSTS_NOT_FOUND:
    case SSH_KNOWN_HOSTS_UNKNOWN:

    }
    int auth_methods = session.getUserAuthList();
    if(SSH_AUTH_METHOD_PUBLICKEY & auth_methods && SSH_AUTH_SUCCESS==session.authenticatePubKey()){
        return auth_result();
    }else if(SSH_AUTH_METHOD_PASSWORD & auth_methods){
        bool ok;
        QString pw = QInputDialog::getText(nullptr, QObject::tr("Remote connection"),QObject::tr("Password:"), QLineEdit::Password,"", &ok);
        if (ok && SSH_AUTH_SUCCESS==session.authenticatePassword(pw)){
            return auth_result();
        }
    }
    return auth_result();
}

} //anon ns

SSHSession::SSHSession(const QString &host, int port)
    : session_(ssh_new())
{
    if(isValid()){
        ssh_options_set(session_, SSH_OPTIONS_HOST, host.toLatin1().data());
        ssh_options_set(session_, SSH_OPTIONS_PORT, &port);
    }
}

SSHSession::SSHSession(const QUrl &url)
    : session_(ssh_new())
{
    if(isValid()){
        ssh_options_set(session_, SSH_OPTIONS_HOST, url.host().toLatin1().data());
        int port=url.port();
        ssh_options_set(session_, SSH_OPTIONS_PORT, &port);
        ssh_options_set(session_, SSH_OPTIONS_USER, url.userName().toLatin1().data());
    }
}

SSHSession::~SSHSession()
{
    if (isValid()){
        disconnect();
        ssh_free(session_);
    }
}

SSHSession SSHSession::createAuthenticatedSession(const QUrl &url)
{
    return authenticate_(url).session;
}

QUrl SSHSession::createAuthenticatedUrl(const QUrl &url)
{
    return authenticate_(url).url;
}

bool SSHSession::isValid() const
{
    return session_!=NULL;
}

bool SSHSession::connect()
{
    if(!isValid()){
        return false;
    }
    int rc = ssh_connect(session_);
    return rc == SSH_OK;
}

void SSHSession::disconnect()
{
    if(! isConnected() ){
        return;
    }
    ssh_disconnect(session_);
}

bool SSHSession::isConnected() const
{
    if(!isValid()){
        return false;
    }
    return ssh_is_connected(session_)==1;
}

QString SSHSession::getError()
{
    if(!isValid()){
        return QString("Invalid SSH session");
    }
    return QString(ssh_get_error(session_));
}

SSHHostVerification SSHSession::authenticateHost()
{
    ssh_key srv_pubkey = NULL;
    SSHHostVerification retval;
    if ((! isConnected()) || (ssh_get_server_publickey(session_, &srv_pubkey) < 0)) {
        return SSHHostVerification{SSH_KNOWN_HOSTS_ERROR,QString()};
    }

    unsigned char *hash = NULL;
    size_t hlen;
    int rc = ssh_get_publickey_hash(srv_pubkey,SSH_PUBLICKEY_HASH_SHA1, &hash, &hlen);
    ssh_key_free(srv_pubkey);
    if (rc < 0) {
        return SSHHostVerification{SSH_KNOWN_HOSTS_ERROR,QString()};
    }

    enum ssh_known_hosts_e state = ssh_session_is_known_server(session_);
    char *hexa;
    switch (state) {
    case SSH_KNOWN_HOSTS_OK:
    case SSH_KNOWN_HOSTS_CHANGED:
    case SSH_KNOWN_HOSTS_OTHER:
    case SSH_KNOWN_HOSTS_NOT_FOUND:
    case SSH_KNOWN_HOSTS_UNKNOWN:
        hexa = ssh_get_hexa(hash, hlen);
        retval = SSHHostVerification{state,QString(hexa)};
        ssh_string_free_char(hexa);
        break;
    case SSH_KNOWN_HOSTS_ERROR:
    default:
        retval = SSHHostVerification{state,QString()};
        break;
     }

    ssh_clean_pubkey_hash(&hash);
    return retval;

}

bool SSHSession::updateKnownHosts()
{
    if(!isConnected()){
        return SSH_ERROR;
    }
    return ssh_session_update_known_hosts(session_) == SSH_OK;
}

int SSHSession::getUserAuthList()
{
    if(!isConnected()){
        return 0;
    }
    int rc = ssh_userauth_none(session_, NULL);
    if (rc == SSH_AUTH_SUCCESS || rc == SSH_AUTH_ERROR) {
        return 0;
    }
    return ssh_userauth_list(session_, NULL);
}

int SSHSession::authenticatePubKey()
{
    if(!isConnected()){
        return SSH_AUTH_ERROR;
    }
    return ssh_userauth_publickey_auto(session_, NULL, NULL);
}

int SSHSession::authenticatePassword(const QString password)
{
    if(!isConnected()){
        return SSH_AUTH_ERROR;
    }
    return ssh_userauth_password(session_, NULL, password.toLatin1().data());
}

int SSHSession::authenticateKbInt()
{
    if(!isConnected()){
        return SSH_AUTH_ERROR;
    }
    return ssh_userauth_kbdint(session_, NULL, NULL);
}

SSHKbdIntPromptList SSHSession::getKbdIntPrompts()
{
    SSHKbdIntPromptList retval;
    if(!isConnected()){
        return retval;
    }

    retval.name = QString(ssh_userauth_kbdint_getname(session_));
    retval.instructions=QString(ssh_userauth_kbdint_getinstruction(session_));
    int nprompts = ssh_userauth_kbdint_getnprompts(session_);

     for(int iprompt = 0; iprompt < nprompts; ++iprompt)
    {
        const char *prompt;
        char echo;

        prompt = ssh_userauth_kbdint_getprompt(session_, iprompt, &echo);
        retval.prompts.append(SSHKbdIntPrompt{prompt,echo!=0});
     }
     return retval;
}

int SSHSession::setKbdIntAnswers(const QStringList &answers)
{
    if(!isConnected()){
        return SSH_AUTH_ERROR;
    }

    for(int iprompt = 0; iprompt < answers.size(); ++iprompt)
    {
        if (ssh_userauth_kbdint_setanswer(session_, iprompt, answers.at(iprompt).toLatin1().data()) < 0){
            return SSH_AUTH_ERROR;
        }
    }
    return ssh_userauth_kbdint(session_, NULL, NULL);
}


