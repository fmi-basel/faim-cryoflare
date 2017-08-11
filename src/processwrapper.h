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

public slots:
    void start(const TaskPtr &task);
    void onFinished(int exitcode);
    void kill();
    void terminate();
    void timeout();
private:
    QProcess *process_;
    TaskPtr task_;
    int timeout_;
    int gpu_id_;
    bool running_;
    QTimer* timeout_timer_;
};

#endif // PROCESSWRAPPER_H
