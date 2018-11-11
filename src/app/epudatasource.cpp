#include <QDir>
#include <QDomDocument>
#include <QDebug>
#include <QDateTime>
#include "epudatasource.h"
#include "filesystemwatcher.h"

EPUDataSource::EPUDataSource():
    DataSourceBase(),
    watcher_(new FileSystemWatcher()),
    epu_project_dir_(),
    movie_dir_(),
    xml_files_(),
    grid_square_data_()
{
    connect(watcher_.data(),&FileSystemWatcher::directoryChanged,this,&EPUDataSource::onDirChanged);
}

EPUDataSource::~EPUDataSource()
{

}

void EPUDataSource::start()
{
    watcher_->addPath(epu_project_dir_);
}

void EPUDataSource::stop()
{
    watcher_->removeAllPaths();
}

void EPUDataSource::setProjectDir(const QString &epu_project_dir)
{
    epu_project_dir_=epu_project_dir;
}

void EPUDataSource::setMovieDir(const QString &movie_dir)
{
    movie_dir_=movie_dir;
}

void EPUDataSource::onDirChanged(const QString &path)
{
    QDir dir(path);
    QString dirname=dir.dirName();
    QStringList watched_directories=watcher_->directories();
    QFileInfoList sub_directories;
    if (path==epu_project_dir_){
        sub_directories=dir.entryInfoList(QStringList("Images-Disc*"),QDir::Dirs,QDir::Time | QDir::Reversed);
    }else if(dirname.startsWith("Images")){
        sub_directories=dir.entryInfoList(QStringList("GridSquare_*"),QDir::Dirs,QDir::Time | QDir::Reversed);
    }else if(dirname.startsWith("GridSquare")){
        sub_directories.append(QFileInfo(dir.absoluteFilePath("Data")));
    }else if(dirname.startsWith("Data")){
        QFileInfoList xml_files=dir.entryInfoList(QStringList("FoilHole*.xml"));
        for(int i=0;i<xml_files.size();++i){
            QString xml_path=xml_files.at(i).absoluteFilePath();
            if(!xml_files_.contains(xml_path)){
                xml_files_.append(xml_path);
                DataPtr data=readXML_(xml_path);
                if(!data.isNull()){
                    emit newImage(data);
                }
                return;
            }
        }
        return;
    }else{
        qInfo() << QString("unknown dir change event: %1").arg(path);
        return;
    }
    for(int i=0;i<sub_directories.size();++i){
        QString abs_path=sub_directories.at(i).absoluteFilePath();
        if(! watched_directories.contains(abs_path) ){
            watcher_->addPath(abs_path);
        }
    }
}

DataPtr EPUDataSource::readXML_(const QString &path)
{
    DataPtr result(new Data());
    QDir xml_dir(path);
    QString name=xml_dir.dirName();
    name.truncate(name.lastIndexOf('.'));

    QDir grid_square_dir(xml_dir);
    grid_square_dir.cdUp();
    grid_square_dir.cdUp();
    QString grid_name=grid_square_dir.dirName();
    QString grid_id=grid_name.remove(0,11);
    if(! grid_square_data_.contains(grid_id)){
        // read grid square xml
        DataPtr grid_data(new Data());
        grid_data->insert("X","0");
        grid_data->insert("Y","0");
        grid_data->insert("Z","0");
        grid_data->insert("A","0");
        grid_data->insert("B","0");
        grid_data->insert("image_shift_x","0");
        grid_data->insert("image_shift_y","0");

        QDomDocument dom_document;
        QStringList grid_sqare_xmls=grid_square_dir.entryList(QStringList("GridSquare*.xml"));
        if(grid_sqare_xmls.isEmpty()){
            qDebug() << "Missing grid square xml for : "+grid_square_dir.absolutePath();
        }else{
            QString xml_path=grid_square_dir.absoluteFilePath(grid_sqare_xmls.at(0));
            QFile file(xml_path);
            if (!file.open(QIODevice::ReadOnly)){
                qDebug() << "Could not open : "+xml_path;
            }else{
                if (!dom_document.setContent(&file)) {
                    qDebug() << "Error parsing : "+xml_path;
                    file.close();
                }else{
                    file.close();
                    QDomNode position=dom_document.elementsByTagName("Position").at(0);
                    grid_data->insert("X",position.toElement().elementsByTagName("X").at(0).toElement().text());
                    grid_data->insert("Y",position.toElement().elementsByTagName("Y").at(0).toElement().text());
                    grid_data->insert("Z",position.toElement().elementsByTagName("Z").at(0).toElement().text());
                    grid_data->insert("A",position.toElement().elementsByTagName("A").at(0).toElement().text());
                    grid_data->insert("B",position.toElement().elementsByTagName("B").at(0).toElement().text());
                    QDomNode image_shift=dom_document.elementsByTagName("ImageShift").at(0);
                    grid_data->insert("image_shift_x",image_shift.toElement().elementsByTagName("a:_x").at(0).toElement().text());
                    grid_data->insert("image_shift_y",image_shift.toElement().elementsByTagName("a:_y").at(0).toElement().text());
                    grid_square_data_.insert(grid_id,grid_data);
                }
            }
        }
    }
    result->insert("square_X",grid_square_data_[grid_id]->value("X"));
    result->insert("square_Y",grid_square_data_[grid_id]->value("Y"));
    result->insert("square_Z",grid_square_data_[grid_id]->value("Z"));
    result->insert("square_A",grid_square_data_[grid_id]->value("A"));
    result->insert("square_B",grid_square_data_[grid_id]->value("B"));
    result->insert("square_image_shift_x",grid_square_data_[grid_id]->value("image_shift_x"));
    result->insert("square_image_shift_y",grid_square_data_[grid_id]->value("image_shift_y"));

    result->insert("xml_file",path);
    result->insert("name",name);
    QStringList splitted_name=name.split("_");
    if(splitted_name.size()<2){
        qDebug() << "Encountered image name not conforming to EPU conventions: "+name;
        return DataPtr();
    }
    result->insert("hole_id",splitted_name.at(1));
    result->insert("grid_name",grid_name);
    result->insert("square_id",grid_name.remove(0,11));
    QDir images_disc_dir(grid_square_dir);
    images_disc_dir.cdUp();
    result->insert("disc_name",images_disc_dir.dirName());
    QDomDocument dom_document;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        qDebug() << "Cannot open XML file: " << path;
        return DataPtr();
    if (!dom_document.setContent(&file)) {
        qDebug() << "Cannot parse XML file: " << path;
        file.close();
        return DataPtr();
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
    result->insert("apix_x",QString("%1").arg(pixel_size_values.at(0).toElement().text().toFloat()*1e10/result->value("super_resolution_factor").toDouble()));
    result->insert("apix_y",QString("%1").arg(pixel_size_values.at(1).toElement().text().toFloat()*1e10/result->value("super_resolution_factor").toDouble()));


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

    QString relative_path=QString("%1/%2/Data").arg(result->value("disc_name").toString()).arg(result->value("grid_name").toString());
    QString avg_s_path=QString("%1/%2").arg(epu_project_dir_).arg(relative_path);
    QString stack_s_path=QString("%1/%2").arg(movie_dir_).arg(relative_path);
    result->insert("destination_path",QDir::currentPath());
    result->insert("stack_source_path",stack_s_path);
    result->insert("avg_source_path",avg_s_path);
    QStringList stack_frames;
    if(QString("BM-Falcon")==result->value("camera").toString()){
        result->insert("stack_frames",QString("%1/%2_frames.mrc").arg(stack_s_path).arg(result->value("name").toString()));
    }else if(QString("EF-CCD")==result->value("camera").toString()){
        for(int i=1;i<=result->value("num_frames").toInt();++i){
            stack_frames.append(QString("%1/%2-*-%3.???").arg(stack_s_path).arg(result->value("name").toString()).arg(i,4,10,QChar('0')));
        }
        result->insert("stack_frames",stack_frames.join(" "));
    }

    return result;

}
