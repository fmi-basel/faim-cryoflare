#ifndef PROCESSWRAPPER_H
#define PROCESSWRAPPER_H

#include <QObject>
#include <QProcess>
#include <QHash>
#include "task.h"

//fw decl
class QTimer;

class ProcessWrapper : public QObject
{
    Q_OBJECT
public:
    explicit ProcessWrapper(QObject *parent, int timeout, int gpu_id);
    bool running() const;

signals:
    void finished(const TaskPtr &task, bool gpu=false);
    void started(const QString &image, const QString &task,int process_id);
    void stopped();

public slots:
    void start(const TaskPtr &task);
    void onFinished(int exitcode);
    void onStarted();
    void kill();
    void terminate();
    void timeout();
private:
    QProcess *process_;
    TaskPtr task_;
    int timeout_;
    bool terminated_;
    int gpu_id_;
    QTimer* timeout_timer_;
  
};

#endif // PROCESSWRAPPER_H
