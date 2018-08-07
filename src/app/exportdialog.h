#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include <QUrl>
#include <QProgressDialog>
#include "../external/qssh/sshconnection.h"

namespace Ui {
class ExportDialog;
}

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(QWidget *parent = 0);
    ~ExportDialog();
    QUrl destinationPath() const;
    QUrl rawDestinationPath() const;
    void setDestinationPath(const QUrl& url);
    void setRawDestinationPath(const QUrl& url);
    bool separateRawPath() const;
    void setSeparateRawPath( bool f);


private:
    Ui::ExportDialog *ui;
};

#endif // EXPORTDIALOG_H
