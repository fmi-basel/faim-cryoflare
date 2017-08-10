#ifndef PROCESSWRAPPER_H
#define PROCESSWRAPPER_H

#include <QObject>
#include <QProcess>
#include <QHash>
#include "task.h"

class ProcessWrapper : public QObject
{
    Q_OBJECT
public:
    explicit ProcessWrapper(QObject *parent, int gpu_id);
    bool running() const;

signals:
    void finished(const TaskPtr &task, bool gpu=false);

public slots:
    void start(const TaskPtr &task);
    void onFinished(int exitcode);
    void kill();
    void terminate();
private:
    QProcess *process_;
    TaskPtr task_;
    int gpu_id_;
    bool running_;
};

#endif // PROCESSWRAPPER_H
