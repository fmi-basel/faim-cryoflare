#include "sshsession.h"
#include "qdebug.h"
#include <QInputDialog>
#include <QObject>
#include <QMessageBox>

#define MAX_AUTH_TRIALS 3

SSHSession::SSHSession()
    : session_(ssh_new(),ssh_free),
    url_()
{
    if(isValid()){
        ssh_set_blocking(session(),1);
    }
}

SSHSession::~SSHSession()
{
}

SSHSession SSHSession::createAuthenticatedSession(const QUrl &url)
{
    SSHSession session;
    if(!session.connect(url)){
        QMessageBox::critical(nullptr,QObject::tr("SSH error during connection"),session.getError());
        return SSHSession();
    }
    SSHHostVerification host_ver=session.authenticateHost();
    switch(host_ver.status){
    case SSH_KNOWN_HOSTS_ERROR:
        QMessageBox::critical(nullptr,QObject::tr("SSH host verification error"),session.getError());
        return SSHSession();
    case SSH_KNOWN_HOSTS_OK:
        break;
    case SSH_KNOWN_HOSTS_CHANGED:
        QMessageBox::critical(nullptr,QObject::tr("SSH host verification changed"),QObject::tr("Host key for server %1 changed: it is now:\n%2\nFor security reasons, connection will be stopped").arg(url.host()).arg(host_ver.hexa));
        return SSHSession();
    case SSH_KNOWN_HOSTS_OTHER:
        QMessageBox::critical(nullptr,QObject::tr("SSH host verification changed"),QObject::tr("The host key for host %1 was not found but an other type of key exists.\nAn attacker might change the default server key to confuse your client into thinking the key does not exist.\nFor security reasons, connection will be stopped.").arg(url.host()));
        return SSHSession();
    case SSH_KNOWN_HOSTS_NOT_FOUND:
        if(QMessageBox::Yes==QMessageBox::question(nullptr, QObject::tr("SSH known hosts missing"), QObject::tr("Could not find know hosts file. Do you trust the host key for host %1?\n%2\nIf you accept the file will be automatically creted").arg(url.host()).arg(host_ver.hexa), QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes)){
            if(!session.updateKnownHosts()){
                QMessageBox::critical(nullptr,QObject::tr("SSH known host update"),QObject::tr("Failed to update known host file. Aborting."));
                return SSHSession();
            }
            break;
        }else{
            return SSHSession();
        }
    case SSH_KNOWN_HOSTS_UNKNOWN:
        if(QMessageBox::Yes==QMessageBox::question(nullptr, QObject::tr("SSH host unknown"), QObject::tr("The server %1 is unknown. Do you trust the host key?\n%2").arg(url.host()).arg(host_ver.hexa), QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes)){
            if(!session.updateKnownHosts()){
                QMessageBox::critical(nullptr,QObject::tr("SSH known host update"),QObject::tr("Failed to update known host file. Aborting."));
                return SSHSession();
            }
            break;
        }else{
            return SSHSession();
        }
    default:
        QMessageBox::critical(nullptr,QObject::tr("SSH host verification error"),QObject::tr("Unknown host verification error (status %1)").arg(host_ver.status));
        return SSHSession();
    }
    int auth_methods = session.getUserAuthList();
    if(SSH_AUTH_METHOD_PUBLICKEY & auth_methods && SSH_AUTH_SUCCESS==session.authenticatePubKey()){
        return session;
    }else if(SSH_AUTH_METHOD_PASSWORD & auth_methods){
        if (SSH_AUTH_SUCCESS==session.authenticatePassword(url.password())){
            return session;
        }
        for(int i=0;i<MAX_AUTH_TRIALS;++i){
            bool ok;
            QString pw = QInputDialog::getText(nullptr, QObject::tr("Remote connection"),QObject::tr("Password:"), QLineEdit::Password,"", &ok);
            if (ok && SSH_AUTH_SUCCESS==session.authenticatePassword(pw)){
                QUrl auth_url(url);
                auth_url.setPassword(pw);
                return session;
            }
        }
    }
    return SSHSession();
}


bool SSHSession::isValid() const
{
    return session_!=NULL;
}

bool SSHSession::connect(const QUrl &url)
{
    if(!isValid()){
        return false;
    }
    if(isConnected()){
        disconnect();
    }
    ssh_options_set(session(), SSH_OPTIONS_HOST, url.host().toLatin1().data());
    int port=url.port();
    ssh_options_set(session(), SSH_OPTIONS_PORT, &port);
    ssh_options_set(session(), SSH_OPTIONS_USER, url.userName().toLatin1().data());
    int rc = ssh_connect(session());
    if(rc == SSH_OK){
        url_=url;
        return true;
    }
    return false;
}

QUrl SSHSession::getUrl() const
{
    return url_;
}

void SSHSession::disconnect()
{
    if(! isConnected() ){
        return;
    }
    ssh_disconnect(session());
}

bool SSHSession::isConnected() const
{
    if(!isValid()){
        return false;
    }
    return ssh_is_connected(session())==1;
}

QString SSHSession::getError()
{
    if(!isValid()){
        return QString("Invalid SSH session");
    }
    return QString(ssh_get_error(session()));
}

SSHHostVerification SSHSession::authenticateHost()
{
    ssh_key srv_pubkey = NULL;
    SSHHostVerification retval;
    if ((! isConnected()) || (ssh_get_server_publickey(session(), &srv_pubkey) < 0)) {
        return SSHHostVerification{SSH_KNOWN_HOSTS_ERROR,QString()};
    }

    unsigned char *hash = NULL;
    size_t hlen;
    int rc = ssh_get_publickey_hash(srv_pubkey,SSH_PUBLICKEY_HASH_SHA1, &hash, &hlen);
    ssh_key_free(srv_pubkey);
    if (rc < 0) {
        return SSHHostVerification{SSH_KNOWN_HOSTS_ERROR,QString()};
    }

    enum ssh_known_hosts_e state = ssh_session_is_known_server(session());
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
    return ssh_session_update_known_hosts(session()) == SSH_OK;
}

int SSHSession::getUserAuthList()
{
    if(!isConnected()){
        return 0;
    }
    int rc = ssh_userauth_none(session(), NULL);
    if (rc == SSH_AUTH_SUCCESS || rc == SSH_AUTH_ERROR) {
        return 0;
    }
    return ssh_userauth_list(session(), NULL);
}

int SSHSession::authenticatePubKey()
{
    if(!isConnected()){
        return SSH_AUTH_ERROR;
    }
    int rc = ssh_userauth_publickey_auto(session(), NULL, NULL);
    if(rc == SSH_AUTH_SUCCESS){
        url_.setPassword(QString());
    }
    return rc;
}

int SSHSession::authenticatePassword(const QString password)
{
    if(!isConnected()){
        return SSH_AUTH_ERROR;
    }
    int rc=ssh_userauth_password(session(), NULL, password.toLatin1().data());
    if(rc == SSH_AUTH_SUCCESS){
        url_.setPassword(password);
    }
    return rc;
}

int SSHSession::authenticateKbInt()
{
    if(!isConnected()){
        return SSH_AUTH_ERROR;
    }
    return ssh_userauth_kbdint(session(), NULL, NULL);
}

SSHKbdIntPromptList SSHSession::getKbdIntPrompts()
{
    SSHKbdIntPromptList retval;
    if(!isConnected()){
        return retval;
    }

    retval.name = QString(ssh_userauth_kbdint_getname(session()));
    retval.instructions=QString(ssh_userauth_kbdint_getinstruction(session()));
    int nprompts = ssh_userauth_kbdint_getnprompts(session());

     for(int iprompt = 0; iprompt < nprompts; ++iprompt)
    {
        const char *prompt;
        char echo;

        prompt = ssh_userauth_kbdint_getprompt(session(), iprompt, &echo);
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
        if (ssh_userauth_kbdint_setanswer(session(), iprompt, answers.at(iprompt).toLatin1().data()) < 0){
            return SSH_AUTH_ERROR;
        }
    }
    return ssh_userauth_kbdint(session(), NULL, NULL);
}

ssh_session SSHSession::session() const
{
    return session_.data();
}


