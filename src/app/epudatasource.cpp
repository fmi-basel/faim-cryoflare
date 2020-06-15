//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2019 by the CryoFLARE Authors
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
// along with CryoFLARE.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------------
#include <QDomDocument>
#include <QDebug>
#include <QDateTime>
#include "readepuxml.h"
#include "epudatasource.h"
#include "filesystemwatcher.h"

//const QString timeformat("yyyy-MM-dd hh:mm:ss.z");


EPUDataSource::EPUDataSource(MetaDataStore *store):
    DataSourceBase(store),
    watcher_(new FileSystemWatcher()),
    epu_project_dir_(),
    movie_dir_(),
    xml_files_(),
    grid_square_data_(),
    foil_hole_data_()
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
    QString relative_path=QDir(epu_project_dir_).relativeFilePath(path);
    QString dirname=dir.dirName();
    QStringList watched_directories=watcher_->directories();
    if (path==epu_project_dir_){
        addSubdirectories_(dir,QStringList() << "Images-Disc*" << "Metadata");
    }else if(relative_path.startsWith("Images-Disc")){
        if(dirname.startsWith("Images-Disc")){
            // Images-Disc_* directory
            addSubdirectories_(dir,QStringList("GridSquare_*"));
        }else if(dirname.startsWith("GridSquare")){
            // grids square directory
            parseGridSquareXMLs_(dir);
            addSubdirectories_(dir,QStringList("Data"));
        }else if(dirname.startsWith("Data")){
            parseMicrographs_(dir);
        }
    }else if(relative_path.startsWith("Metadata")){
        if(dirname.startsWith("Metadata")){
            // Metadata directory
            // parse dm for 2.7 style target locations
            parseGridSquareDMs_(dir);
            // watch directories for 2.6 style target locations
            addSubdirectories_(dir,QStringList("GridSquare_*"));
        }else if(dirname.startsWith("GridSquare")){
            // parse 2.6 style target locations
            parseTargetLocations_(dir);
        }
    }
}

void EPUDataSource::parseGridSquareXMLs_(const QDir &dir)
{
    QString square_id=dir.dirName();
    square_id.remove(0,11);
    QStringList grid_sqare_xmls=dir.entryList(QStringList("GridSquare*.xml"));
    if(grid_sqare_xmls.isEmpty()){
        qDebug() << "Missing grid square xml for : "+dir.absolutePath();
        return;
    }
    QString xml_path=dir.absoluteFilePath(grid_sqare_xmls.at(0));
    QDateTime last_modified=QFileInfo(xml_path).lastModified();
    Data grid_data;
    if(grid_square_data_.contains(square_id)){
        QDateTime previous_modified=QDateTime::fromString(grid_square_data_.value(square_id).value("last_modified").toString(),timeformat);
        if(last_modified <= previous_modified){
            return;
        }
        grid_data=grid_square_data_.value(square_id);
    }
    // read grid square xml
    grid_data.insert("X","0");
    grid_data.insert("Y","0");
    grid_data.insert("Z","0");
    grid_data.insert("A","0");
    grid_data.insert("B","0");
    grid_data.insert("image_shift_x","0");
    grid_data.insert("image_shift_y","0");

    QFile file(xml_path);
    if (!file.open(QIODevice::ReadOnly)){
        qDebug() << "Could not open : "+xml_path;
        return;
    }
    QDomDocument dom_document;
    if (!dom_document.setContent(&file)) {
        qDebug() << "Error parsing : "+xml_path;
        file.close();
        return;
    }
    file.close();
    grid_data.insert("id",square_id);
    QDomNode position=dom_document.elementsByTagName("Position").at(0);
    grid_data.insert("X",position.toElement().elementsByTagName("X").at(0).toElement().text());
    grid_data.insert("Y",position.toElement().elementsByTagName("Y").at(0).toElement().text());
    grid_data.insert("Z",position.toElement().elementsByTagName("Z").at(0).toElement().text());
    grid_data.insert("A",position.toElement().elementsByTagName("A").at(0).toElement().text());
    grid_data.insert("B",position.toElement().elementsByTagName("B").at(0).toElement().text());
    QDomNode image_shift=dom_document.elementsByTagName("ImageShift").at(0);
    grid_data.insert("image_shift_x",image_shift.toElement().elementsByTagName("a:_x").at(0).toElement().text());
    grid_data.insert("image_shift_y",image_shift.toElement().elementsByTagName("a:_y").at(0).toElement().text());
    grid_data.insert("micrograph",xml_path.replace(".xml",".mrc"));
    grid_data.insert("last_modified",last_modified.toString(timeformat));
    grid_square_data_.insert(square_id,grid_data);
    emit newGridsquare(grid_data);
}

void EPUDataSource::parseGridSquareDMs_(const QDir &dir)
{
    // read 2.7 foil hole metadata
    foreach(QString file_entry, dir.entryList(QStringList("GridSquare*.dm"),QDir::Files,QDir::Name)){
        QString meta_path=dir.filePath(file_entry);
        QString square_id=file_entry.remove(0,11);
        QFile meta_file(meta_path);
        QDateTime last_modified=QFileInfo(meta_file).lastModified();
        if(grid_square_data_.contains(square_id)){
            Data square_data=grid_square_data_.value(square_id);
            QDateTime previous_modified=QDateTime::fromString(square_data.value("last_modified").toString(),timeformat);
            if(last_modified <= previous_modified){
                continue;
            }
            square_data.insert("last_modified",last_modified.toString(timeformat));
            grid_square_data_.insert(square_id,square_data);
        }
        if (!meta_file.open(QIODevice::ReadOnly)){
            qDebug() << "Cannot open grid square meta data: " << meta_path;
            continue;
        }
        QDomDocument meta_dom_document;
        if (!meta_dom_document.setContent(&meta_file)) {
            qDebug() << "Cannot parse grid square meta data: " << meta_path;
            meta_file.close();
            continue;
        }
        meta_file.close();

        QDomNode target_location_array=meta_dom_document.elementsByTagName("a:m_serializationArray").at(0);
        QDomNode target_node = target_location_array.firstChild();
        while(!target_node.isNull()) {
            QString hole_id=target_node.toElement().elementsByTagName("c:TargetLocationId").at(0).toElement().text();
            Data data;
            if(foil_hole_data_.contains(hole_id)){
                data=foil_hole_data_.value(hole_id);
                QDateTime previous_modified=QDateTime::fromString(data.value("last_modified").toString(),timeformat);
                if(last_modified <= previous_modified){
                    target_node = target_node.nextSibling();
                    continue;
                }
            }
            data.insert("id",hole_id);
            data.insert("square_id",square_id);
            QString x=target_node.toElement().elementsByTagName("c:x").at(0).toElement().text();
            data.insert("x",x);
            QString y=target_node.toElement().elementsByTagName("c:y").at(0).toElement().text();
            data.insert("y",y);
            QString selected=target_node.toElement().elementsByTagName("Selected").at(0).toElement().text();
            data.insert("selected",selected);
            target_node = target_node.nextSibling();
            data.insert("last_modified",last_modified.toString(timeformat));
            foil_hole_data_.insert(hole_id,data);
            emit newFoilhole(data);
        }
    }
}

void EPUDataSource::parseTargetLocations_(const QDir &dir)
{
    // read 2.6 foil hole metadata
    QString square_id=dir.dirName();
    square_id.remove(0,11);
    foreach(QString file_entry, dir.entryList(QStringList("TargetLocation_*.dm"),QDir::Files,QDir::Name)){
        Data data;
        QString id=file_entry.remove(0,15).remove(8,4);
        QString fh_meta_path=dir.filePath(file_entry);
        QFile fh_meta_file(fh_meta_path);
        QDateTime last_modified=QFileInfo(fh_meta_file).lastModified();
        if(foil_hole_data_.contains(id)){
            QDateTime previous_modified=QDateTime::fromString(foil_hole_data_.value(square_id).value("last_modified").toString(),timeformat);
            if(last_modified <= previous_modified){
                continue;
            }
            data=foil_hole_data_.value(id);
        }
        if (!fh_meta_file.open(QIODevice::ReadOnly)){
            qDebug() << "Cannot open Foil hole meta data: " << fh_meta_path;
            continue;
        }
        QDomDocument fh_meta_dom_document;
        if (!fh_meta_dom_document.setContent(&fh_meta_file)) {
            qDebug() << "Cannot parse foil hole meta data: " << fh_meta_path;
            fh_meta_file.close();
            continue;
        }
        fh_meta_file.close();
        QString x=fh_meta_dom_document.elementsByTagName("a:x").at(0).toElement().text();
        data.insert("x",x);
        QString y=fh_meta_dom_document.elementsByTagName("a:y").at(0).toElement().text();
        data.insert("y",y);
        QString selected=fh_meta_dom_document.elementsByTagName("Selected").at(0).toElement().text();
        data.insert("selected",selected);
        data.insert("id",id);
        data.insert("square_id",square_id);
        data.insert("last_modified",last_modified.toString(timeformat));
        foil_hole_data_.insert(id,data);
        emit newFoilhole(data);
    }
}

void EPUDataSource::parseMicrographs_(const QDir &dir)
{
    QFileInfoList xml_files=dir.entryInfoList(QStringList("FoilHole*.xml"));
    for(int i=0;i<xml_files.size();++i){
        QString xml_path=xml_files.at(i).absoluteFilePath();
        if(!xml_files_.contains(xml_path)){
            xml_files_.append(xml_path);
            Data data=readEPUXML(xml_path);
            if(data.empty()){
                continue;
            }
            QDir xml_dir(xml_path);
            QString hole_id=xml_dir.dirName().split("_")[1];
            QDir grid_square_dir(xml_dir);
            grid_square_dir.cdUp();
            grid_square_dir.cdUp();
            QString square_id=grid_square_dir.dirName();
            square_id.remove(0,11);
            data.insert("square_id",square_id);
            QDir images_disc_dir(grid_square_dir);
            images_disc_dir.cdUp();
            data.insert("disc_name",images_disc_dir.dirName());

            if(grid_square_data_.contains(square_id)){
                data.insert("square_X",grid_square_data_[square_id].value("X"));
                data.insert("square_Y",grid_square_data_[square_id].value("Y"));
                data.insert("square_Z",grid_square_data_[square_id].value("Z"));
                data.insert("square_A",grid_square_data_[square_id].value("A"));
                data.insert("square_B",grid_square_data_[square_id].value("B"));
                data.insert("square_image_shift_x",grid_square_data_[square_id].value("image_shift_x"));
                data.insert("square_image_shift_y",grid_square_data_[square_id].value("image_shift_y"));
            }else{
                data.insert("square_X","0");
                data.insert("square_Y","0");
                data.insert("square_Z","0");
                data.insert("square_A","0");
                data.insert("square_B","0");
                data.insert("square_image_shift_x","0");
                data.insert("square_image_shift_y","0");
            }
            if(foil_hole_data_.contains(hole_id)){
                data.insert("hole_pos_x",foil_hole_data_.value(hole_id).value("x"));
                data.insert("hole_pos_y",foil_hole_data_.value(hole_id).value("y"));
            }else{
                data.insert("hole_pos_x","0");
                data.insert("hole_pos_y","0");
            }
            QString relative_path=QString("%1/GridSquare_%2/Data").arg(data.value("disc_name").toString()).arg(data.value("square_id").toString());
            QString avg_s_path=QString("%1/%2").arg(epu_project_dir_).arg(relative_path);
            QString stack_s_path=QString("%1/%2").arg(movie_dir_).arg(relative_path);
            data.insert("destination_path",QDir::currentPath());
            data.insert("stack_source_path",stack_s_path);
            data.insert("avg_source_path",avg_s_path);
            QStringList stack_frames;
            if(QString("BM-Falcon")==data.value("camera").toString()){
                data.insert("stack_frames",QString("%1/%2_frames.mrc").arg(stack_s_path).arg(data.value("name").toString()));
            }else if(QString("EF-CCD")==data.value("camera").toString()){
                for(int i=1;i<=data.value("num_frames").toInt();++i){
                    stack_frames.append(QString("%1/%2-*-%3.???").arg(stack_s_path).arg(data.value("name").toString()).arg(i,4,10,QChar('0')));
                }
                data.insert("stack_frames",stack_frames.join(" "));
            }
            emit newMicrograph(data);
        }
    }
}

void EPUDataSource::addSubdirectories_(const QDir &directory, const QStringList &subdirs)
{
    QStringList watched_directories=watcher_->directories();
    QFileInfoList sub_directories=directory.entryInfoList(subdirs,QDir::Dirs,QDir::Time | QDir::Reversed);
    for(int i=0;i<sub_directories.size();++i){
        QString abs_path=sub_directories.at(i).absoluteFilePath();
        if(! watched_directories.contains(abs_path) ){
            watcher_->addPath(abs_path);
        }
    }
}

