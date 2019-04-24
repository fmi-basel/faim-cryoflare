#include <QDomDocument>
#include <QDir>
#include <QDebug>
#include <QDateTime>
#include "readepuxml.h"
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
                DataPtr data=readEPUXML(xml_path);
                if(data.isNull()){
                    continue;
                }
                QDir xml_dir(xml_path);
                QDir grid_square_dir(xml_dir);
                grid_square_dir.cdUp();
                grid_square_dir.cdUp();
                QString square_id=grid_square_dir.dirName();
                square_id.remove(0,11);
                data->insert("square_id",square_id);
                QDir images_disc_dir(grid_square_dir);
                images_disc_dir.cdUp();
                data->insert("disc_name",images_disc_dir.dirName());
                if(! grid_square_data_.contains(square_id)){
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
                                grid_square_data_.insert(square_id,grid_data);
                            }
                        }
                    }
                }
                if(grid_square_data_.contains(square_id)){
                    data->insert("square_X",grid_square_data_[square_id]->value("X"));
                    data->insert("square_Y",grid_square_data_[square_id]->value("Y"));
                    data->insert("square_Z",grid_square_data_[square_id]->value("Z"));
                    data->insert("square_A",grid_square_data_[square_id]->value("A"));
                    data->insert("square_B",grid_square_data_[square_id]->value("B"));
                    data->insert("square_image_shift_x",grid_square_data_[square_id]->value("image_shift_x"));
                    data->insert("square_image_shift_y",grid_square_data_[square_id]->value("image_shift_y"));
                }else{
                    data->insert("square_X","0");
                    data->insert("square_Y","0");
                    data->insert("square_Z","0");
                    data->insert("square_A","0");
                    data->insert("square_B","0");
                    data->insert("square_image_shift_x","0");
                    data->insert("square_image_shift_y","0");
                }
                // read foil hole metadata
                QDomDocument fh_meta_dom_document;
                QString fh_meta_path=QString("%1/Metadata/GridSquare_%2/TargetLocation_%3.dm").arg(epu_project_dir_).arg(data->value("square_id").toString()).arg(data->value("hole_id").toString());
                QFile fh_meta_file(fh_meta_path);
                if (!fh_meta_file.open(QIODevice::ReadOnly)){
                    qDebug() << "Cannot open Foil hole meta data: " << fh_meta_path;
                    data->insert("hole_pos_x","0");
                    data->insert("hole_pos_y","0");
                }else{
                    if (!fh_meta_dom_document.setContent(&fh_meta_file)) {
                        qDebug() << "Cannot parse foil hole meta data: " << fh_meta_path;
                        data->insert("hole_pos_x","0");
                        data->insert("hole_pos_y","0");
                    }else{
                        QString hole_pos_x=fh_meta_dom_document.elementsByTagName("a:x").at(0).toElement().text();
                        data->insert("hole_pos_x",hole_pos_x);
                        QString hole_pos_y=fh_meta_dom_document.elementsByTagName("a:y").at(0).toElement().text();
                        data->insert("hole_pos_y",hole_pos_y);
                    }
                }
                fh_meta_file.close();

                QString relative_path=QString("%1/GridSquare_%2/Data").arg(data->value("disc_name").toString()).arg(data->value("square_id").toString());
                QString avg_s_path=QString("%1/%2").arg(epu_project_dir_).arg(relative_path);
                QString stack_s_path=QString("%1/%2").arg(movie_dir_).arg(relative_path);
                data->insert("destination_path",QDir::currentPath());
                data->insert("stack_source_path",stack_s_path);
                data->insert("avg_source_path",avg_s_path);
                QStringList stack_frames;
                if(QString("BM-Falcon")==data->value("camera").toString()){
                    data->insert("stack_frames",QString("%1/%2_frames.mrc").arg(stack_s_path).arg(data->value("name").toString()));
                }else if(QString("EF-CCD")==data->value("camera").toString()){
                    for(int i=1;i<=data->value("num_frames").toInt();++i){
                        stack_frames.append(QString("%1/%2-*-%3.???").arg(stack_s_path).arg(data->value("name").toString()).arg(i,4,10,QChar('0')));
                    }
                    data->insert("stack_frames",stack_frames.join(" "));
                }

                emit newImage(data);
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

