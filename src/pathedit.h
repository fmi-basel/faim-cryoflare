#ifndef PATHEDIT_H
#define PATHEDIT_H

#include <QWidget>

//fw decl
class QLineEdit;
class QPushButton;

class PathEdit : public QWidget
{
    Q_OBJECT
public:
    enum PathType{
        ExistingDirectory,
        OpenFileName,
        SaveFileName
    };
    explicit PathEdit(PathType t=OpenFileName, QString caption=QString(), QString path=QString(), QString filter=QString(), QWidget *parent = 0);
    QString path() const;
    void setPath(const QString &path);

signals:
    void pathChanged(QString);
public slots:
    void onBrowse();

private:
    PathType path_type_;
    QString caption_;
    QString path_;
    QString filter_;
    QLineEdit *path_widget_;
    QPushButton *browse_;

};

#endif // PATHEDIT_H
