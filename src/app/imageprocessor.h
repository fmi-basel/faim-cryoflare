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
class ParallelExporter;
class Settings;
class QProcess;

class ImageProcessor: public QObject
{
    Q_OBJECT
public:
    ImageProcessor();
    ~ImageProcessor();
public slots:
    void startStop(bool start=true);
    void onDirChange(const QString & path);
    void onTaskFinished(const TaskPtr& task, bool gpu);
    void loadSettings();
    void exportImages(const QString& export_path,const QStringList& image_list);
    void startTasks();
signals:
    void newImage(DataPtr data);
    void dataChanged(DataPtr data);
    void imageUpdated(const QString& image);
    void queueCountChanged(int,int);
    void processCreated(ProcessWrapper *wrapper,int gpu_id);
    void processesDeleted();

private:
    void updateDisc_(const QString& disc);
    void updateGridSquare_(const QString& grid_square);
    void updateImages_(const QString& grid_square);
    void createTaskTree_(const QString& path);
    void loadTask_(Settings *setting,const TaskPtr& task);

    FileSystemWatcher* watcher_;
    QString avg_source_path_;
    QString stack_source_path_;
    QStringList grid_squares_;
    QStringList images_;
    QStack<TaskPtr> cpu_task_stack_;
    QStack<TaskPtr> gpu_task_stack_;
    QList<ProcessWrapper*> cpu_processes_;
    QList<ProcessWrapper*> gpu_processes_;
    TaskPtr root_task_;
    QHash<QString,QSet<QString> > raw_files_;
    QHash<QString,QSet<QString> > output_files_;
    QSet<QString> shared_output_files_;
    ParallelExporter* exporter_;
    QProcess* process_;
    bool running_state_;


};


#endif // IMAGEPROCESSOR_H