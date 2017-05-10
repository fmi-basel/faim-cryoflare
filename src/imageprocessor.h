#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QStack>
#include <QObject>
#include <QStringList>
#include <task.h>
#include <dataptr.h>

//fw decl
class FileSystemWatcher;
class ProcessWrapper;
class QSettings;

class ImageProcessor: public QObject
{
    Q_OBJECT
public:
    ImageProcessor();
public slots:
    void startStop(bool start=true);
    void onFileChange(const QString & path);
    void onDirChange(const QString & path);
    void onCPUTaskFinished(const TaskPtr& task);
    void onGPUTaskFinished(const TaskPtr& task);
    void loadSettings();
    void init();
signals:
    void newImage(DataPtr data);
    void dataChanged(DataPtr data);
    void imageUpdated(const QString& image);

private:
    void updateGridSquare_(const QString& grid_square);
    void updateImages_(const QString& grid_square);
    void createTask_(const QString& path);
    void pushTask_(const TaskPtr& task);
    void loadTask_(QSettings *setting,const TaskPtr& task);

    FileSystemWatcher* watcher_;
    QString avg_source_path_;
    QString stack_source_path_;
    QString destination_path_;
    QStringList grid_squares_;
    QStringList images_;
    QStack<TaskPtr> cpu_task_stack_;
    QStack<TaskPtr> gpu_task_stack_;
    QList<ProcessWrapper*> cpu_processes_;
    QList<ProcessWrapper*> gpu_processes_;
    TaskPtr root_task_;


};


#endif // IMAGEPROCESSOR_H
