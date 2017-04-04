#include <QDir>
#include <QSettings>
#include <QtDebug>
#include <filesystemwatcher.h>
#include <QDomDocument>
#include <processwrapper.h>
#include "imageprocessor.h"

namespace  {

DataPtr parse_xml_data(const QString& xml_path){
    DataPtr result(new Data());
    QDir xml_dir(xml_path);
    QString name=xml_dir.dirName();
    name.truncate(name.lastIndexOf('.'));
    xml_dir.cdUp();
    xml_dir.cdUp();
    QString grid_name=xml_dir.dirName();
    result->insert("xml_file",xml_path);
    result->insert("name",name);
    result->insert("grid_name",grid_name);
    QDomDocument dom_document;
    QFile file(xml_path);
    if (!file.open(QIODevice::ReadOnly))
        return result;
    if (!dom_document.setContent(&file)) {
        file.close();
        return result;
    }
    file.close();

    QDomNode custom_data=dom_document.elementsByTagName("CustomData").at(0);
    QDomNode node = custom_data.firstChild();
    while(!node.isNull()) {
        if(node.firstChild().toElement().text()=="AppliedDefocus"){
            result->insert("defocus",node.lastChild().toElement().text());
            break;
        }
        node = node.nextSibling();
    }

    QDomNode camera=dom_document.elementsByTagName("camera").at(0);
    result->insert("num_frames",camera.toElement().elementsByTagName("b:NumberOffractions").at(0).toElement().text());
    result->insert("exposure_time",camera.toElement().elementsByTagName("ExposureTime").at(0).toElement().text());

    QDomNode pixel_size=dom_document.elementsByTagName("pixelSize").at(0);
    QDomNodeList pixel_size_values=pixel_size.toElement().elementsByTagName("numericValue");
    result->insert("apix_x",pixel_size_values.at(0).toElement().text());
    result->insert("apix_y",pixel_size_values.at(1).toElement().text());

    QDomNode stage=dom_document.elementsByTagName("stage").at(0);
    result->insert("x",stage.toElement().elementsByTagName("X").at(0).toElement().text());
    result->insert("y",stage.toElement().elementsByTagName("Y").at(0).toElement().text());
    result->insert("z",stage.toElement().elementsByTagName("Z").at(0).toElement().text());
    
    QDomNode datetime=dom_document.elementsByTagName("acquisitionDateTime").at(0);
    result->insert("timestamp",datetime.toElement().text());
    return result;
}

}

ImageProcessor::ImageProcessor():
    QObject(),
    watcher_(new FileSystemWatcher),
    avg_source_path_(),
    stack_source_path_(),
    destination_path_(),
    grid_squares_(),
    images_(),
    cpu_task_stack_(),
    gpu_task_stack_(),
    cpu_processes_(),
    gpu_processes_(),
    root_task_(new Task("General","dummy",DataPtr(new Data)))
{
    loadSettings();
    connect(watcher_, SIGNAL(fileChanged(const QString &)), this, SLOT(onFileChange(const QString &)));
    connect(watcher_, SIGNAL(directoryChanged(const QString &)), this, SLOT(onDirChange(const QString &)));
    QSettings settings;
    int num_cpu=settings.value("num_cpu",10).toInt();
    int num_gpu=settings.value("num_gpu",2).toInt();
    for(int i=0;i<num_cpu;++i) {
        ProcessWrapper* wrapper=new ProcessWrapper(this,-1);
        connect(wrapper,SIGNAL(finished(TaskPtr)),this,SLOT(onCPUTaskFinished(TaskPtr)));
        cpu_processes_.append(wrapper);

    }
    for(int i=0;i<num_gpu;++i) {
        ProcessWrapper* wrapper=new ProcessWrapper(this,i);
        connect(wrapper,SIGNAL(finished(TaskPtr)),this,SLOT(onGPUTaskFinished(TaskPtr)));
        gpu_processes_.append(wrapper);
    }
}

void ImageProcessor::startStop(bool start)
{
    if(start){
        watcher_->addPath(avg_source_path_);
        watcher_->addPath(stack_source_path_);
        onDirChange(avg_source_path_);
    }else{
        watcher_->removePath(avg_source_path_);
        watcher_->removePath(stack_source_path_);
    }
}


void ImageProcessor::setAvgSourcePath(const QString &path)
{
    avg_source_path_=QDir(path).absoluteFilePath("Images-Disc1");
}

void ImageProcessor::setStackSourcePath(const QString& path)
{
    stack_source_path_=QDir(path).absoluteFilePath("Images-Disc1");
}

void ImageProcessor::setDestinationPath(const QString &path)
{
    destination_path_=path;
}

void ImageProcessor::onFileChange(const QString &path)
{
}

void ImageProcessor::onDirChange(const QString &path)
{
    if(path==avg_source_path_){
        QFileInfoList grid_squares=QDir(avg_source_path_).entryInfoList(QStringList("GridSquare_*"),QDir::Dirs,QDir::Time | QDir::Reversed);
        for(int i=0;i<grid_squares.size();++i){
            updateGridSquare_(grid_squares.at(i).absoluteFilePath());
        }
    }else if(path==stack_source_path_){

    }else if(avg_source_path_==QFileInfo(path).absolutePath()){
        // changes within gridsquare dir
        updateGridSquare_(path);
    }else{
        updateImages_(path);
    }
}

void ImageProcessor::onCPUTaskFinished(const TaskPtr &task)
{
    foreach(TaskPtr child,task->children){
        pushTask_(child);
    }
    if(!cpu_task_stack_.empty()){
        ProcessWrapper* wrapper = qobject_cast<ProcessWrapper*>(sender());
        if( wrapper != NULL && (! wrapper->running()) ) {
              wrapper->start(cpu_task_stack_.pop());
        }
    }
    QFile f(destination_path_+"/"+task->name+"_out.log");
    if (f.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream( &f );
        stream << task->output << endl;
    }
    QFile ferr(destination_path_+"/"+task->name+"_error.log");
    if (ferr.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream( &f );
        stream << task->error << endl;
    }
    emit dataChanged(task->data);
}

void ImageProcessor::onGPUTaskFinished(const TaskPtr &task)
{
    foreach(TaskPtr child,task->children){
        pushTask_(child);
    }
    if(!gpu_task_stack_.empty()){
        ProcessWrapper* wrapper = qobject_cast<ProcessWrapper*>(sender());
        if( wrapper != NULL && (! wrapper->running())) {
              wrapper->start(gpu_task_stack_.pop());
        }
    }
    QFile f(destination_path_+"/"+task->name+"_out.log");
    if (f.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream( &f );
        stream << task->output << endl;
    }
    QFile ferr(destination_path_+"/"+task->name+"_error.log");
    if (ferr.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream( &f );
        stream << task->error << endl;
    }
    emit dataChanged(task->data);
}

void ImageProcessor::loadSettings()
{
    root_task_->children.clear();
    QSettings *settings=new QSettings;
    settings->beginGroup("Tasks");
    loadTask_(settings,root_task_);
    settings->endGroup();
    delete settings;

}

void ImageProcessor::init()
{
}

void ImageProcessor::updateGridSquare_(const QString &grid_square)
{
    QString grid_square_data=QDir(grid_square).absoluteFilePath("Data");
    if(! grid_squares_.contains(grid_square_data)){
        if(QFileInfo(grid_square_data).exists()){
            watcher_->addPath(grid_square_data);
            grid_squares_.append(grid_square_data);
            updateImages_(grid_square_data);
            watcher_->removePath(grid_square);
        }else{
            watcher_->addPath(grid_square);
        }
    }
}

void ImageProcessor::updateImages_(const QString &grid_square)
{
    QFileInfoList xml_files=QDir(grid_square).entryInfoList(QStringList("FoilHole*.xml"));
    for(int i=0;i<xml_files.size();++i){
        if(!images_.contains(xml_files.at(i).absoluteFilePath())){
            images_.append(xml_files.at(i).absoluteFilePath());
            createTask_(xml_files.at(i).absoluteFilePath());
        }
    }
}

void ImageProcessor::createTask_(const QString &path)
{
    DataPtr data=parse_xml_data(path);
    data->insert("destination_path",destination_path_);
    data->insert("stack_source_path",stack_source_path_);
    QStringList stack_frames;
    for(int i=1;i<=data->value("num_frames").toInt();++i){
        stack_frames.append(QString("%1/%2/Data/%3-*-%4.???").arg(stack_source_path_).arg(data->value("grid_name")).arg(data->value("name")).arg(i,4,10,QChar('0')));
    }
    data->insert("stack_frames",stack_frames.join(" "));
    emit newImage(data);
    TaskPtr root_task=root_task_->clone();
    root_task->setData(data);
    for(int i=0;i<root_task->children.size();++i){
        pushTask_(root_task->children[i]);
    }
}

void ImageProcessor::pushTask_(const TaskPtr &task)
{
    if(task->gpu){
        gpu_task_stack_.append(task);
        foreach (ProcessWrapper* proc, gpu_processes_) {
           if(! (proc->running() || gpu_task_stack_.empty())){
               proc->start(gpu_task_stack_.pop());
               break;
           }
        }
    }else{
        cpu_task_stack_.append(task);
        foreach (ProcessWrapper* proc, cpu_processes_) {
           if(! (proc->running()|| cpu_task_stack_.empty())){
               proc->start(cpu_task_stack_.pop());
               break;
           }
        }
    }
}

void ImageProcessor::loadTask_(QSettings *settings, const TaskPtr &task)
{
    foreach(QString child_name,settings->childGroups()){
        settings->beginGroup(child_name);
        TaskPtr child(new Task(child_name,settings->value("script").toString(), DataPtr(new Data()),settings->value("is_gpu").toBool()));
        task->children.append(child);
        loadTask_(settings,child);
        settings->endGroup();
    }

}

