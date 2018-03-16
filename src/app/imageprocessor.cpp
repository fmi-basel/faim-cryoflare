#include <iostream>
#include <QApplication>
#include <QDir>
#include <QDateTime>
#include "settings.h"
#include <QtDebug>
#include <filesystemwatcher.h>
#include <QDomDocument>
#include <QProcess>
#include <processwrapper.h>
#include <parallelexporter.h>
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
    xml_dir.cdUp();
    result->insert("disc_name",xml_dir.dirName());
    QDomDocument dom_document;
    QFile file(xml_path);
    if (!file.open(QIODevice::ReadOnly))
        return result;
    if (!dom_document.setContent(&file)) {
        file.close();
        return result;
    }
    file.close();
    //create fake grid square positions for testing, until acces to real data from xml is available
    result->insert("phase_plate_count","0");
    static int grid_n=0;
    int grid_square=grid_n/(64*4)+1;
    int grid_square_pos=(grid_n-(grid_square-1)*64*4)/4+1;
    int grid_square_hole_pos=grid_n-(grid_square-1)*64*4-(grid_square_pos-1)*4+1;
    result->insert("grid_square",QString("%1").arg(grid_square));
    result->insert("grid_square_pos",QString("%1").arg(grid_square_pos));
    result->insert("grid_square_hole_pos",QString("%1").arg(grid_square_hole_pos));
    ++grid_n;
    QDomNode custom_data=dom_document.elementsByTagName("CustomData").at(0);
    QDomNode node = custom_data.firstChild();
    while(!node.isNull()) {
        if(node.firstChild().toElement().text()=="AppliedDefocus"){
            result->insert("defocus",QString("%1").arg(node.lastChild().toElement().text().toDouble()*1.0e10));
        }   else if(node.firstChild().toElement().text()=="PhasePlateUsed"){
            result->insert("phase_plate",node.lastChild().toElement().text());
        }    else if(node.firstChild().toElement().text()=="Dose"){
            result->insert("dose",node.lastChild().toElement().text());
        }    else if(node.firstChild().toElement().text()=="PhasePlateApertureName"){
            QString phase_plate_str=node.lastChild().toElement().text().split(" ").last();
            result->insert("phase_plate_num",phase_plate_str.right(phase_plate_str.size()-1));
        }    else if(node.firstChild().toElement().text()=="PhasePlatePosition"){
            result->insert("phase_plate_pos",node.lastChild().toElement().text());
        }


        node = node.nextSibling();
    }

    QDomNode camera=dom_document.elementsByTagName("camera").at(0);
    QString camera_name=camera.toElement().elementsByTagName("Name").at(0).toElement().text();
    result->insert("camera",camera_name);
    if(QString("BM-Falcon")==camera_name){
        result->insert("num_frames",QString("%1").arg(camera.toElement().elementsByTagName("b:DoseFractions").at(0).childNodes().size()));
    }else if (QString("EF-CCD")==camera_name){
        result->insert("num_frames",camera.toElement().elementsByTagName("b:NumberOffractions").at(0).toElement().text());
    }
    result->insert("exposure_time",camera.toElement().elementsByTagName("ExposureTime").at(0).toElement().text());

    QDomNode pixel_size=dom_document.elementsByTagName("pixelSize").at(0);
    QDomNodeList pixel_size_values=pixel_size.toElement().elementsByTagName("numericValue");
    result->insert("apix_x",pixel_size_values.at(0).toElement().text());
    result->insert("apix_y",pixel_size_values.at(1).toElement().text());

    QDomNode stage=dom_document.elementsByTagName("stage").at(0);
    result->insert("x",stage.toElement().elementsByTagName("X").at(0).toElement().text());
    result->insert("y",stage.toElement().elementsByTagName("Y").at(0).toElement().text());
    result->insert("z",stage.toElement().elementsByTagName("Z").at(0).toElement().text());
    
    QDomNode nominal_magnification=dom_document.elementsByTagName("NominalMagnification").at(0);
    result->insert("nominal_magnification",nominal_magnification.toElement().text());
    QDomNode datetime=dom_document.elementsByTagName("acquisitionDateTime").at(0);
    QString timestamp=datetime.toElement().text();
    QDateTime time=QDateTime::fromString(timestamp,Qt::ISODate);
    result->insert("timestamp",time.toString("yyyy-MM-dd hh:mm:ss"));
    result->insert("short_name",time.toString("yyyyMMdd_hhmmss"));
    return result;
}

}

ImageProcessor::ImageProcessor():
    QObject(),
    watcher_(new FileSystemWatcher),
    avg_source_path_(),
    stack_source_path_(),
    grid_squares_(),
    images_(),
    cpu_task_stack_(),
    gpu_task_stack_(),
    cpu_processes_(),
    gpu_processes_(),
    root_task_(new Task("General","dummy",DataPtr(new Data))),
    raw_files_(),
    output_files_(),
    shared_output_files_(),
    exporter_(new ParallelExporter(this)),
    process_(new QProcess(this)),
    running_state_(false)

{
    QTimer::singleShot(0, this, SLOT(loadSettings()));
    connect(watcher_, SIGNAL(directoryChanged(const QString &)), this, SLOT(onDirChange(const QString &)));
}

ImageProcessor::~ImageProcessor()
{
    foreach (ProcessWrapper* process, cpu_processes_) {
        process->terminate();
    }
    foreach (ProcessWrapper* process, gpu_processes_) {
        process->terminate();
    }
}

void ImageProcessor::startStop(bool start)
{
    if(start){
        raw_files_.clear();
        output_files_.clear();
        shared_output_files_.clear();
        Settings settings;
        running_state_=true;
        avg_source_path_=settings.value("avg_source_dir").toString();
        stack_source_path_=settings.value("stack_source_dir").toString();
        watcher_->addPath(avg_source_path_);
        onDirChange(avg_source_path_);
        startTasks();
    }else{
        running_state_=false;
        watcher_->removePath(avg_source_path_);
        cpu_task_stack_.clear();
        gpu_task_stack_.clear();
        foreach(ProcessWrapper* process, cpu_processes_){
            process->terminate();
        }
        foreach(ProcessWrapper* process, gpu_processes_){
            process->terminate();
        }
        grid_squares_.clear();
        images_.clear();
    }
}



void ImageProcessor::onDirChange(const QString &path)
{
    QDir dir(path);
    if(dir.absolutePath()==avg_source_path_){
        QFileInfoList images_discs=dir.entryInfoList(QStringList("Images-Disc*"),QDir::Dirs,QDir::Time | QDir::Reversed);
        for(int i=0;i<images_discs.size();++i){
            updateDisc_(images_discs.at(i).absoluteFilePath());
        }
        return;
    }
    dir.cdUp();
    if(dir.absolutePath()==avg_source_path_){
        //Images-Disc* directory has changed
        updateDisc_(path);
        return;
    }
    dir.cdUp();
    if(dir.absolutePath()==avg_source_path_){
        // changes within gridsquare dir
        updateGridSquare_(path);
        return;
    }
    dir.cdUp();
    if(dir.absolutePath()==avg_source_path_){
        updateImages_(path);
        return;
    }
}

void ImageProcessor::onTaskFinished(const TaskPtr &task, bool gpu)
{
    if(! output_files_.contains(task->data->value("short_name"))){
        output_files_[task->data->value("short_name")]=QSet<QString>();
    }
    foreach(QString file, task->output_files){
        output_files_[task->data->value("short_name")].insert(QDir::current().relativeFilePath(file));
    }
    if(! raw_files_.contains(task->data->value("short_name"))){
        raw_files_[task->data->value("short_name")]=QSet<QString>();
    }
    foreach(QString file, task->raw_files){
        raw_files_[task->data->value("short_name")].insert(QDir::current().relativeFilePath(file));
    }
    foreach(QString file, task->shared_output_files){
        shared_output_files_.insert(QDir::current().relativeFilePath(file));
    }
    foreach(TaskPtr child,task->children){
        QStack<TaskPtr>& stack=child->gpu?gpu_task_stack_:cpu_task_stack_;
        stack.append(child);
    }
    emit queueCountChanged(cpu_task_stack_.size(),gpu_task_stack_.size());
    startTasks();
    QFile f(QDir::current().relativeFilePath(task->name+"_out.log"));
    if (f.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream( &f );
        stream << task->output << endl;
    }
    QFile ferr(QDir::current().relativeFilePath(task->name+"_error.log"));
    if (ferr.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream( &f );
        stream << task->error << endl;
    }
    emit dataChanged(task->data);
}


void ImageProcessor::loadSettings()
{
     emit processesDeleted();
     Settings *settings=new Settings;
     while(!cpu_processes_.empty()){
         cpu_processes_.takeLast()->deleteLater();
     }
     while(!gpu_processes_.empty()){
         gpu_processes_.takeLast()->deleteLater();
     }
     int num_cpu=settings->value("num_cpu",10).toInt();
     int num_gpu=settings->value("num_gpu",2).toInt();
     int timeout=settings->value("timeout",300).toInt();
     QStringList gpu_ids=settings->value("gpu_ids","0").toString().split(",", QString::SkipEmptyParts);
     if(gpu_ids.empty()){
         for(int i=0;i<num_gpu;++i) {
             gpu_ids << QString("%1").arg(i);
         }
     }
     for(int i=0;i<num_cpu;++i) {
         ProcessWrapper* wrapper=new ProcessWrapper(this,timeout,-1);
         emit processCreated(wrapper,-1);
         connect(wrapper,SIGNAL(finished(TaskPtr,bool)),this,SLOT(onTaskFinished(TaskPtr,bool)));
         cpu_processes_.append(wrapper);

     }
     for(int i=0;i<num_gpu;++i) {
         int gpu_id=gpu_ids.at(i%gpu_ids.size()).toInt();
         ProcessWrapper* wrapper=new ProcessWrapper(this,timeout,gpu_id);
         emit processCreated(wrapper,gpu_id);
         connect(wrapper,SIGNAL(finished(TaskPtr,bool)),this,SLOT(onTaskFinished(TaskPtr,bool)));
         gpu_processes_.append(wrapper);
     }

    root_task_->children.clear();
    settings->beginGroup("Tasks");
    loadTask_(settings,root_task_);
    settings->endGroup();
    delete settings;

}

void ImageProcessor::exportImages(const QString &export_path, const QStringList &image_list)
{
    QQueue<QSet<QString> >files_to_export;
    QQueue<QSet<QString> >raw_files_to_export;
    files_to_export.enqueue(shared_output_files_);
    foreach(QString image,image_list){
        files_to_export.enqueue(output_files_[image]);
        raw_files_to_export.enqueue(raw_files_[image]);
    }
    Settings settings;
    QString export_mode=settings.value("export").toString();
    QString custom_script=settings.value("export_custom_script").toString();
    int num_processes=settings.value("export_num_processes",1).toInt();
    QString pre_script=settings.value("export_pre_script").toString();
    QString post_script=settings.value("export_post_script").toString();
    if(export_mode=="custom2"){
        process_->setProcessChannelMode(QProcess::MergedChannels);
        process_->setStandardOutputFile(QDir(QDir::currentPath()).absoluteFilePath("export.log"));
        QStringList arguments;
        arguments << QDir::currentPath();
        process_->start(custom_script,arguments);
        process_->waitForStarted(-1);
        foreach(QString image,image_list){
            qDebug() << "gui writing: " << QStringList(raw_files_[image].toList()).join(",") << "\n";
            qDebug() << "gui writing: " << QStringList(output_files_[image].toList()).join(",") << "\n";
            process_->write(QString("raw_%1=%2\n").arg(image).arg(QStringList(raw_files_[image].toList()).join(",")).toLatin1());
            process_->write(QString("%1=%2\n").arg(image).arg(QStringList(output_files_[image].toList()).join(",")).toLatin1());
        }
        process_->write(QString("%1=%2\n").arg("shared").arg(QStringList(shared_output_files_.toList()).join(",")).toLatin1());
        qDebug() << "gui data written\n";
        process_->closeWriteChannel();
        //process.waitForFinished(-1);

    }else{
        exporter_->exportImages(QDir::currentPath() , export_path, files_to_export, num_processes, export_mode, custom_script,pre_script,post_script,image_list);
    }

}

void ImageProcessor::startTasks()
{
    if (! running_state_){
        return;
    }
    bool count_changed=false;
    foreach (ProcessWrapper* proc, cpu_processes_) {
       if(! proc->running() &&  ! cpu_task_stack_.empty()){
           proc->start(cpu_task_stack_.pop());
           count_changed=true;
       }
    }
    foreach (ProcessWrapper* proc, gpu_processes_) {
       if(! proc->running() &&  ! gpu_task_stack_.empty()){
           proc->start(gpu_task_stack_.pop());
           count_changed=true;
       }
    }
    if(count_changed){
        emit queueCountChanged(cpu_task_stack_.size(),gpu_task_stack_.size());
    }
}

void ImageProcessor::updateDisc_(const QString &disc)
{
    QFileInfoList grid_squares=QDir(disc).entryInfoList(QStringList("GridSquare_*"),QDir::Dirs,QDir::Time | QDir::Reversed);
    for(int i=0;i<grid_squares.size();++i){
        updateGridSquare_(grid_squares.at(i).absoluteFilePath());
    }
    return;
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
            createTaskTree_(xml_files.at(i).absoluteFilePath());
        }
    }
}

void ImageProcessor::createTaskTree_(const QString &path)
{
    DataPtr data=parse_xml_data(path);
    QString relative_path=QString("%1/%2/Data").arg(data->value("disc_name")).arg(data->value("grid_name"));
    QString avg_s_path=QString("%1/%2").arg(avg_source_path_).arg(relative_path);
    QString stack_s_path=QString("%1/%2").arg(stack_source_path_).arg(relative_path);
    data->insert("destination_path",QDir::currentPath());
    data->insert("stack_source_path",stack_s_path);
    data->insert("avg_source_path",avg_s_path);
    QStringList stack_frames;
    if(QString("BM-Falcon")==data->value("camera")){
        data->insert("stack_frames",QString("%1/%2_frames.mrc").arg(stack_s_path).arg(data->value("name")));
    }else if(QString("EF-CCD")==data->value("camera")){
        for(int i=1;i<=data->value("num_frames").toInt();++i){
            stack_frames.append(QString("%1/%2-*-%3.???").arg(stack_s_path).arg(data->value("name")).arg(i,4,10,QChar('0')));
        }
        data->insert("stack_frames",stack_frames.join(" "));
    }
    emit newImage(data);
    TaskPtr root_task=root_task_->clone();
    root_task->setData(data);
    for(int i=0;i<root_task->children.size();++i){
        QStack<TaskPtr>& stack=root_task->children[i]->gpu?gpu_task_stack_:cpu_task_stack_;
        stack.append(root_task->children[i]);
    }
    emit queueCountChanged(cpu_task_stack_.size(),gpu_task_stack_.size());
    startTasks();
}


void ImageProcessor::loadTask_(Settings *settings, const TaskPtr &task)
{
    foreach(QString child_name,settings->childGroups()){
        settings->beginGroup(child_name);
        TaskPtr child(new Task(child_name,settings->value("script").toString(), DataPtr(new Data()),settings->value("is_gpu").toBool()));
        task->children.append(child);
        loadTask_(settings,child);
        settings->endGroup();
    }

}

