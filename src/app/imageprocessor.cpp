//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFlare
//
// Copyright (C) 2017-2018 by the CryoFlare Authors
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3.0 of the License.
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License
// along with CryoFlare.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------------

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
#include "imageprocessor.h"

namespace  {

DataPtr parse_grid_xml(const QDir& grid_square_dir){
    DataPtr result(new Data());
    result->insert("X","0");
    result->insert("Y","0");
    result->insert("Z","0");
    result->insert("A","0");
    result->insert("B","0");
    result->insert("image_shift_x","0");
    result->insert("image_shift_y","0");

    QDomDocument dom_document;
    QStringList grid_sqare_xmls=grid_square_dir.entryList(QStringList("GridSquare*.xml"));
    if(grid_sqare_xmls.isEmpty()){
        qDebug() << "Missing grid square xml for : "+grid_square_dir.absolutePath();
        return result;
    }
    QString xml_path=grid_square_dir.absoluteFilePath(grid_sqare_xmls.at(0));
    QFile file(xml_path);
    if (!file.open(QIODevice::ReadOnly)){
        qDebug() << "Could not open : "+xml_path;
        return result;
    }
    if (!dom_document.setContent(&file)) {
        qDebug() << "Error parsing : "+xml_path;
        file.close();
        return result;
    }
    file.close();
    QDomNode position=dom_document.elementsByTagName("Position").at(0);
    result->insert("X",position.toElement().elementsByTagName("X").at(0).toElement().text());
    result->insert("Y",position.toElement().elementsByTagName("Y").at(0).toElement().text());
    result->insert("Z",position.toElement().elementsByTagName("Z").at(0).toElement().text());
    result->insert("A",position.toElement().elementsByTagName("A").at(0).toElement().text());
    result->insert("B",position.toElement().elementsByTagName("B").at(0).toElement().text());
    QDomNode image_shift=dom_document.elementsByTagName("ImageShift").at(0);
    result->insert("image_shift_x",image_shift.toElement().elementsByTagName("a:_x").at(0).toElement().text());
    result->insert("image_shift_y",image_shift.toElement().elementsByTagName("a:_y").at(0).toElement().text());
    return result;
}

DataPtr parse_xml_data(const QString& xml_path){
    DataPtr result(new Data());
    QDir xml_dir(xml_path);
    QString name=xml_dir.dirName();
    name.truncate(name.lastIndexOf('.'));
    xml_dir.cdUp();
    xml_dir.cdUp();
    DataPtr square_data=parse_grid_xml(xml_dir);
    result->insert("square_X",square_data->value("X"));
    result->insert("square_Y",square_data->value("Y"));
    result->insert("square_Z",square_data->value("Z"));
    result->insert("square_A",square_data->value("A"));
    result->insert("square_B",square_data->value("B"));
    result->insert("square_image_shift_x",square_data->value("image_shift_x"));
    result->insert("square_image_shift_y",square_data->value("image_shift_y"));

    QString grid_name=xml_dir.dirName();
    result->insert("xml_file",xml_path);
    result->insert("name",name);
    QStringList splitted_name=name.split("_");
    if(splitted_name.size()<2){
        qDebug() << "Encountered image name not conforming to EPU conventions: "+name;
        QCoreApplication::exit(-1);
    }
    result->insert("hole_id",splitted_name.at(1));
    result->insert("grid_name",grid_name);
    result->insert("square_id",grid_name.remove(0,11));
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
    QDomNode custom_data=dom_document.elementsByTagName("CustomData").at(0);
    QDomNode node = custom_data.firstChild();
    while(!node.isNull()) {
        if(node.firstChild().toElement().text()=="AppliedDefocus"){
            result->insert("defocus",QString("%1").arg(node.lastChild().toElement().text().toDouble()*1.0e10));
        }   else if(node.firstChild().toElement().text()=="PhasePlateUsed"){
            result->insert("phase_plate",node.lastChild().toElement().text());
        }    else if(node.firstChild().toElement().text()=="Dose"){
            result->insert("dose",QString("%1").arg(node.lastChild().toElement().text().toDouble()*1.0e-20));
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
    QDomNode cam_specific_input=dom_document.elementsByTagName("CameraSpecificInput").at(0);
    QDomNodeList inputs=cam_specific_input.childNodes();
    result->insert("super_resolution_factor","1");
    for(int i=0;i<inputs.size();++i){
        QDomNode input=inputs.at(i);
        QString key=input.firstChild().toElement().text();
        QString value=input.lastChild().toElement().text();
        if(key=="SuperResolutionFactor"){
            result->insert("super_resolution_factor",value);
        }
    } 
    result->insert("apix_x",QString("%1").arg(pixel_size_values.at(0).toElement().text().toFloat()*1e10/result->value("super_resolution_factor").toFloat()));
    result->insert("apix_y",QString("%1").arg(pixel_size_values.at(1).toElement().text().toFloat()*1e10/result->value("super_resolution_factor").toFloat()));

    
    result->insert("acceleration_voltage",QString("%1").arg(dom_document.elementsByTagName("AccelerationVoltage").at(0).toElement().text().toFloat()/1000.0));
    QDomNode nominal_magnification=dom_document.elementsByTagName("NominalMagnification").at(0);
    result->insert("nominal_magnification",nominal_magnification.toElement().text());
    QDomNode datetime=dom_document.elementsByTagName("acquisitionDateTime").at(0);
    QString timestamp=datetime.toElement().text();
    QDateTime time=QDateTime::fromString(timestamp,Qt::ISODate);
    result->insert("timestamp",time.toString("yyyy-MM-dd hh:mm:ss"));
    result->insert("short_name",time.toString("yyyyMMdd_hhmmss"));
    QDomNode position=dom_document.elementsByTagName("Position").at(0);
    result->insert("X",position.toElement().elementsByTagName("X").at(0).toElement().text());
    result->insert("Y",position.toElement().elementsByTagName("Y").at(0).toElement().text());
    result->insert("Z",position.toElement().elementsByTagName("Z").at(0).toElement().text());
    result->insert("A",position.toElement().elementsByTagName("A").at(0).toElement().text());
    result->insert("B",position.toElement().elementsByTagName("B").at(0).toElement().text());
    QDomNode image_shift=dom_document.elementsByTagName("ImageShift").at(0);
    result->insert("image_shift_x",image_shift.toElement().elementsByTagName("a:_x").at(0).toElement().text());
    result->insert("image_shift_y",image_shift.toElement().elementsByTagName("a:_y").at(0).toElement().text());

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
    exporters_(),
    current_exporter_(nullptr),
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
        watcher_->removeAllPaths();
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
        output_files_[task->data->value("short_name")]=QMap<QString,QString>();
    }
    foreach(QString key, task->output_files.keys()){
        output_files_[task->data->value("short_name")].insert(key,QDir::current().relativeFilePath(task->output_files.value(key)));
    }
    if(! raw_files_.contains(task->data->value("short_name"))){
        raw_files_[task->data->value("short_name")]=QMap<QString,QString>();
    }
    foreach(QString key, task->raw_files.keys()){
        raw_files_[task->data->value("short_name")].insert(key,QDir::current().relativeFilePath(task->raw_files.value(key)));
    }
    foreach(QString key, task->shared_output_files.keys()){
        shared_output_files_.insert(key,QDir::current().relativeFilePath( task->shared_output_files.value(key)));
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
    task->data->insert("tasks_unfinished",QString("%1").arg(task->data->value("tasks_unfinished","1").toInt()-1));
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

void ImageProcessor::exportImages(const SftpUrl &export_path, const SftpUrl &raw_export_path, const QStringList &image_list, const QStringList &output_keys, const QStringList &raw_keys, const QStringList &shared_keys, bool duplicate_raw)
{
    Settings settings;
    QString export_mode=settings.value("export").toString();
    if(export_mode=="custom2"){
        QString custom_script=settings.value("export_custom_script").toString();
        QFileInfo check_file(custom_script);
        if (!check_file.exists() || !check_file.isFile()) {
	    qInfo() << "Custom export script doesn't exist:" << custom_script;
            return;
        }
        process_->setProcessChannelMode(QProcess::MergedChannels);
        process_->setStandardOutputFile(QDir(QDir::currentPath()).absoluteFilePath("export.log"));
        QStringList arguments;
        arguments << QDir::currentPath();
        process_->start(custom_script,arguments);
        process_->waitForStarted(-1);
        foreach(QString image,image_list){
            QStringList raw_list;
            foreach(QString key, raw_files_[image].keys()){
                if(raw_keys.contains(key)){
                    raw_list << raw_files_[image][key];
                }
            }
            process_->write(QString("raw_%1=%2\n").arg(image).arg(raw_list.join(",")).toLatin1());
            QStringList output_list;
            foreach(QString key, output_files_[image].keys()){
                if(output_keys.contains(key)){
                    output_list << output_files_[image][key];
                }
            }
            process_->write(QString("%1=%2\n").arg(image).arg(output_list.join(",")).toLatin1());
        }
        QStringList shared_list;
        foreach(QString key, shared_output_files_.keys()){
            if(shared_keys.contains(key)){
                shared_list << shared_output_files_[key];
            }
        }
        process_->write(QString("%1=%2\n").arg("shared").arg(shared_list.join(",")).toLatin1());
        process_->closeWriteChannel();
    }else{
        bool separate_raw_export=export_path!=raw_export_path;
        int num_processes=settings.value("export_num_processes",1).toInt();

        QStringList files;
        QStringList raw_files;
        QStringList files_to_filter;
        QString current_dir=QDir::current().dirName();
        foreach(QString key,shared_output_files_.keys()){
            if(shared_output_files_[key].endsWith(".star")){
                files_to_filter.append(current_dir+"/"+shared_output_files_[key]);
            }else{
                files.append(current_dir+"/"+shared_output_files_[key]);
            }
        }
        foreach(QString image,image_list){
            foreach(QString key, output_files_[image].keys()){
                if(output_keys.contains(key)){
                    files << current_dir+"/"+output_files_[image][key];
                }
            }
            foreach(QString key, raw_files_[image].keys()){
                if(raw_keys.contains(key)){
                    if( separate_raw_export ){
                        raw_files << current_dir+"/"+raw_files_[image][key];
                        if( duplicate_raw){
                            files << current_dir+"/"+raw_files_[image][key];
                        }
                    }else{
                        files << current_dir+"/"+raw_files_[image][key];
                    }
                }
            }
        }
        QDir parent_dir=QDir::current();
        parent_dir.cdUp();
        if(!files.empty() || !files_to_filter.empty()){
            qDebug() << files_to_filter;
            ParallelExporter* exporter=new ParallelExporter(parent_dir.path(),export_path,image_list,num_processes);
            exporter->addImages(files_to_filter,true);
            exporter->addImages(files,false);
            exporters_.enqueue(exporter);
        }
        if(separate_raw_export && !raw_files.empty()){
            ParallelExporter* raw_exporter=new ParallelExporter(parent_dir.path(),raw_export_path,image_list,num_processes);
            raw_exporter->addImages(raw_files,false);
            exporters_.enqueue(raw_exporter);
        }
        startNextExport_();
    }

}

void ImageProcessor::cancelExport()
{
    if(current_exporter_){
        current_exporter_->cancel();
        exportFinished_();
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

QSet<QString> ImageProcessor::getOutputFilesKeys() const
{
    QSet<QString> result;
    foreach(QString hash_key, output_files_.keys()){
        result.unite(QSet<QString>::fromList(output_files_[hash_key].keys()));
    }
    return result;
}

QSet<QString> ImageProcessor::getRawFilesKeys() const
{
    QSet<QString> result;
    foreach(QString hash_key, raw_files_.keys()){
        result.unite(QSet<QString>::fromList(raw_files_[hash_key].keys()));
    }
    return result;
}

QSet<QString> ImageProcessor::getSharedFilesKeys() const
{
    return QSet<QString>::fromList(shared_output_files_.keys());
}

void ImageProcessor::exportFinished_()
{
    delete current_exporter_;
    current_exporter_=nullptr;
    emit exportFinished();
    startNextExport_();
}

void ImageProcessor::startNextExport_()
{
    if(current_exporter_){
        // don't start new export if one is already running
        return;
    }
    if(!exporters_.empty()){
        current_exporter_=exporters_.dequeue();
        //finished signal needs to go to GUI first, before current exporter is deleted
        connect(current_exporter_,&ParallelExporter::finished,this,&ImageProcessor::exportFinished_);
        connect(current_exporter_,&ParallelExporter::message,this,&ImageProcessor::exportMessage);
        current_exporter_->start();
        emit exportStarted(current_exporter_->destination().toString(QUrl::RemovePassword),current_exporter_->numFiles());
    }else{
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
        if(QFileInfo::exists(grid_square_data)){
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
        data->insert("stack_frames",QString("%1/%2_frames.???").arg(stack_s_path).arg(data->value("name")));
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
        QStack<TaskPtr>& stack=root_task->children.at(i)->gpu?gpu_task_stack_:cpu_task_stack_;
        stack.append(root_task->children.at(i));
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

