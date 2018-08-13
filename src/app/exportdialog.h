#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include <QProgressDialog>
#include "../external/qssh/sshconnection.h"
#include "sftpurl.h"

namespace Ui {
class ExportDialog;
}

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(QWidget *parent = nullptr);
    ~ExportDialog();
    SftpUrl destinationPath() const;
    SftpUrl rawDestinationPath() const;
    void setDestinationPath(const QUrl& url);
    void setRawDestinationPath(const QUrl& url);
    bool separateRawPath() const;
    void setSeparateRawPath( bool f);
public slots:
    void verifyDestinations();
    void destinationVerified();
    void destinationVerificationError(QSsh::SshError e);
    void rawDestinationVerificationError(QSsh::SshError e);


private:
    Ui::ExportDialog *ui;
    QSsh::SshConnection* connection_;
};

#endif // EXPORTDIALOG_H
