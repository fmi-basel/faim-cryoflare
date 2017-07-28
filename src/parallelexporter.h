#ifndef PARALLELEXPORTER_H
#define PARALLELEXPORTER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QQueue>
#include <QMutex>
#include <QSet>

//fw decl
class QProgressDialog;

class Worker : public QObject
{
    Q_OBJECT
public:
    Worker(const QString &source, const QString &destination, QQueue<QSet<QString> > *queue, QMutex *mutex, const QString &mode, const QString &script, QObject *parent = 0);
    ~Worker();

public slots:
    void process();

signals:
    void nextImage();
    void finished();
protected:
    QString source_;
    QString destination_;
    QQueue<QSet<QString> > *queue_;
    QMutex *mutex_;
    QString mode_;
    QString script_;
};

class ParallelExporter : public QObject
{
    Q_OBJECT
public:
    explicit ParallelExporter(QObject *parent = 0);
    void exportImages(const QString &source, const QString &destination, QQueue<QSet<QString> > &image_list, int num_processes=1, const QString& mode=QString("copy"), const QString& custom_script=QString(""), const QString& pre_script=QString(""), const QString& post_script=QString(""),const QStringList& image_names=QStringList());
signals:

public slots:
    void updateProgress();
    void cancel();
    void runPost();
protected:
    QQueue<QSet<QString> > queue_;
    int num_tasks_;
    int num_tasks_done_;
    QProgressDialog* dialog_;
    QMutex mutex_;
    int num_threads_;
    QString post_script_;
    QStringList post_arguments_;
};

#endif // PARALLELEXPORTER_H
