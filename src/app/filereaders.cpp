#include "filereaders.h"
#include "datafolderwatcher.h"
#include <QDebug>
#include <QDateTime>
#include <QDir>
#include <QDomDocument>
#include <QJsonDocument>
#include <QPair>

DataFolderWatcher *createEPUFolderWatcher(QObject *parent)
{
    DataFolderWatcher * watcher=new DataFolderWatcher(parent);
    FolderNode data;
    data.pattern=QRegularExpression("^Data$");
    data.file_readers.append(QPair<QRegularExpression,FileReaderPtr>(QRegularExpression("^FoilHole_.*\\.xml$"),readEPUMicrographXML));
    FolderNode im_grid_square;
    im_grid_square.pattern=QRegularExpression("^GridSquare_.*");
    im_grid_square.file_readers.append(QPair<QRegularExpression,FileReaderPtr>(QRegularExpression("GridSquare_.*\\.xml$"),readEPUGridSquareXML));
    im_grid_square.children.append(data);
    FolderNode images_disc;
    images_disc.pattern=QRegularExpression("^Images-Disc*.*$");
    images_disc.children.append(im_grid_square);


    FolderNode meta_grid_square;
    meta_grid_square.pattern=QRegularExpression("^GridSquare_.*$");
    meta_grid_square.file_readers.append(QPair<QRegularExpression,FileReaderPtr>(QRegularExpression("^TargetLocation_.*\\.dm$"),readEPUTargetLocationDM));
    FolderNode metadata;
    metadata.pattern=QRegularExpression("^Metadata$");
    metadata.file_readers.append(QPair<QRegularExpression,FileReaderPtr>(QRegularExpression("^GridSquare_.*\\.dm$"),readEPUGridSquareDM));
    metadata.children.append(meta_grid_square);

    FolderNode root;
    root.children.append(images_disc);
    root.children.append(metadata);
    watcher->setRootFolder(root);
    return watcher;
}

ParsedData readEPUMicrographXML(const QFileInfo &info, const QString& project_dir, const QString &movie_dir)
{
    ParsedData data;
    Data result;
    QString path=info.absoluteFilePath();
    QDir xml_dir(path);
    QString name=xml_dir.dirName();
    name.truncate(name.lastIndexOf('.'));
    result.insert("xml_file",path);
    result.insert("name",name);
    QStringList splitted_name=name.split("_");
    QDomDocument dom_document;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)){
        qDebug() << "Cannot open XML file: " << path;
        return data;
    }
    if (!dom_document.setContent(&file)) {
        qDebug() << "Cannot parse XML file: " << path;
        file.close();
        return data;
    }
    file.close();
    QDomNode custom_data=dom_document.elementsByTagName("CustomData").at(0);
    QDomNode node = custom_data.firstChild();
    while(!node.isNull()) {
        if(node.firstChild().toElement().text()=="AppliedDefocus"){
            result.insert("defocus",QString("%1").arg(node.lastChild().toElement().text().toDouble()*1.0e10));
        }   else if(node.firstChild().toElement().text()=="PhasePlateUsed"){
            result.insert("phase_plate",node.lastChild().toElement().text());
        }    else if(node.firstChild().toElement().text()=="Dose"){
            result.insert("dose",QString("%1").arg(node.lastChild().toElement().text().toDouble()*1.0e-20));
        }    else if(node.firstChild().toElement().text()=="PhasePlateApertureName"){
            QString phase_plate_str=node.lastChild().toElement().text().split(" ").last();
            result.insert("phase_plate_num",phase_plate_str.right(phase_plate_str.size()-1));
        }    else if(node.firstChild().toElement().text()=="PhasePlatePosition"){
            result.insert("phase_plate_pos",node.lastChild().toElement().text());
        }
        node = node.nextSibling();
    }
    QDomNode camera=dom_document.elementsByTagName("camera").at(0);
    QString camera_name=camera.toElement().elementsByTagName("Name").at(0).toElement().text();
    result.insert("camera",camera_name);
    if(QString("BM-Falcon")==camera_name){
        result.insert("num_frames",QString("%1").arg(camera.toElement().elementsByTagName("b:DoseFractions").at(0).childNodes().size()));
    }else if (QString("EF-CCD")==camera_name){
        result.insert("num_frames",camera.toElement().elementsByTagName("b:NumberOffractions").at(0).toElement().text());
    }
    result.insert("exposure_time",camera.toElement().elementsByTagName("ExposureTime").at(0).toElement().text());
    QDomNode pixel_size=dom_document.elementsByTagName("pixelSize").at(0);
    QDomNodeList pixel_size_values=pixel_size.toElement().elementsByTagName("numericValue");
    QDomNode cam_specific_input=dom_document.elementsByTagName("CameraSpecificInput").at(0);
    QDomNodeList inputs=cam_specific_input.childNodes();
    result.insert("super_resolution_factor","1");
    for(int i=0;i<inputs.size();++i){
        QDomNode input=inputs.at(i);
        QString key=input.firstChild().toElement().text();
        QString value=input.lastChild().toElement().text();
        if(key=="SuperResolutionFactor"){
            result.insert("super_resolution_factor",value.toDouble());
        }
    }
    result.insert("apix_x",QString("%1").arg(pixel_size_values.at(0).toElement().text().toDouble()*1e10/result.value("super_resolution_factor").toDouble()));
    result.insert("apix_y",QString("%1").arg(pixel_size_values.at(1).toElement().text().toDouble()*1e10/result.value("super_resolution_factor").toDouble()));
    result.insert("acceleration_voltage",QString("%1").arg(dom_document.elementsByTagName("AccelerationVoltage").at(0).toElement().text().toDouble()/1000.0));
    QDomNode nominal_magnification=dom_document.elementsByTagName("NominalMagnification").at(0);
    result.insert("nominal_magnification",nominal_magnification.toElement().text());
    QDomNode datetime=dom_document.elementsByTagName("acquisitionDateTime").at(0);
    QString timestamp=datetime.toElement().text();
    QDateTime time=QDateTime::fromString(timestamp,Qt::ISODate);
    result.insert("timestamp",time.toString("yyyy-MM-dd hh:mm:ss"));
    result.insert("short_name",time.toString("yyyyMMdd_hhmmss"));
    QString id=time.toString("yyyyMMdd_hhmmss");
    result.setId(id);
    QDomNode position=dom_document.elementsByTagName("Position").at(0);
    result.insert("x",position.toElement().elementsByTagName("X").at(0).toElement().text());
    result.insert("y",position.toElement().elementsByTagName("Y").at(0).toElement().text());
    result.insert("z",position.toElement().elementsByTagName("Z").at(0).toElement().text());
    result.insert("a",position.toElement().elementsByTagName("A").at(0).toElement().text());
    result.insert("b",position.toElement().elementsByTagName("B").at(0).toElement().text());
    QDomNode image_shift=dom_document.elementsByTagName("ImageShift").at(0);
    result.insert("image_shift_x",image_shift.toElement().elementsByTagName("a:_x").at(0).toElement().text());
    result.insert("image_shift_y",image_shift.toElement().elementsByTagName("a:_y").at(0).toElement().text());
    QDomNode beam_shift=dom_document.elementsByTagName("BeamShift").at(0);
    result.insert("beam_shift_x",beam_shift.toElement().elementsByTagName("a:_x").at(0).toElement().text());
    result.insert("beam_shift_y",beam_shift.toElement().elementsByTagName("a:_y").at(0).toElement().text());
    QString avg_s_path= info.absoluteDir().absolutePath();
    QString relative_path=QDir(project_dir).relativeFilePath(avg_s_path);
    QString stack_s_path=QString("%1/%2").arg(movie_dir).arg(relative_path);
    result.insert("destination_path",QDir::currentPath());
    result.insert("stack_source_path",stack_s_path);
    result.insert("avg_source_path",avg_s_path);
    QStringList stack_frames;
    if(QString("BM-Falcon")==result.value("camera").toString()){
        result.insert("stack_frames",QString("%1/%2_frames.mrc").arg(stack_s_path).arg(result.value("name").toString()));
    }else if(QString("EF-CCD")==result.value("camera").toString()){
        for(int i=1;i<=result.value("num_frames").toInt();++i){
            stack_frames.append(QString("%1/%2-*-%3.???").arg(stack_s_path).arg(result.value("name").toString()).arg(i,4,10,QChar('0')));
        }
        result.insert("stack_frames",stack_frames.join(" "));
    }
    if(splitted_name.size()>=7){
        //original EPU file not from flat folder
        QString fh_id=splitted_name.at(1);
        result.setParent(fh_id);
        Data fh_data;
        fh_data.setId(fh_id);
        fh_data.addChild(id);
        data.foil_holes.append(fh_data);
        result.insert("template_id",splitted_name.at(3));
        result.insert("acquisition_id",splitted_name.at(4));
    }

    data.micrographs.append(result);
    return data;
}

ParsedData readEPUGridSquareXML(const QFileInfo &info, const QString& project_dir, const QString &movie_dir)
{
    ParsedData result;
    QString square_id=info.dir().dirName();
    square_id.remove(0,11);
    QString xml_path=info.absoluteFilePath();
    QDateTime last_modified=QFileInfo(xml_path).lastModified();
    Data grid_data;
    // read grid square xml
    grid_data.insert("x","0");
    grid_data.insert("y","0");
    grid_data.insert("z","0");
    grid_data.insert("a","0");
    grid_data.insert("b","0");
    grid_data.insert("image_shift_x","0");
    grid_data.insert("image_shift_y","0");
    QFile file(xml_path);
    if (!file.open(QIODevice::ReadOnly)){
        qDebug() << "Could not open : "+xml_path;
        return result;
    }
    QDomDocument dom_document;
    if (!dom_document.setContent(&file)) {
        qDebug() << "Error parsing : "+xml_path;
        file.close();
        return result;
    }
    file.close();
    grid_data.setId(square_id);
    QDomNode position=dom_document.elementsByTagName("Position").at(0);
    grid_data.insert("x",position.toElement().elementsByTagName("X").at(0).toElement().text());
    grid_data.insert("y",position.toElement().elementsByTagName("Y").at(0).toElement().text());
    grid_data.insert("z",position.toElement().elementsByTagName("Z").at(0).toElement().text());
    grid_data.insert("a",position.toElement().elementsByTagName("A").at(0).toElement().text());
    grid_data.insert("b",position.toElement().elementsByTagName("B").at(0).toElement().text());
    QDomNode image_shift=dom_document.elementsByTagName("ImageShift").at(0);
    grid_data.insert("image_shift_x",image_shift.toElement().elementsByTagName("a:_x").at(0).toElement().text());
    grid_data.insert("image_shift_y",image_shift.toElement().elementsByTagName("a:_y").at(0).toElement().text());
    grid_data.insert("micrograph",xml_path.replace(".xml",".mrc"));
    grid_data.setTimestamp(last_modified);
    result.grid_squares.append(grid_data);
    return result;
}

ParsedData readEPUTargetLocationDM(const QFileInfo &info, const QString& project_dir, const QString &movie_dir)
{
    // read 2.6 foil hole metadata

    QString square_id=info.dir().dirName();
    square_id.remove(0,11);

    Data data;
    ParsedData result;
    QString id=info.fileName().remove(0,15).remove(8,4);
    QString fh_meta_path=info.absoluteFilePath();
    QFile fh_meta_file(fh_meta_path);
    QDateTime last_modified=QFileInfo(fh_meta_file).lastModified();
    if (!fh_meta_file.open(QIODevice::ReadOnly)){
        qDebug() << "Cannot open Foil hole meta data: " << fh_meta_path;
        return result;
    }
    QDomDocument fh_meta_dom_document;
    if (!fh_meta_dom_document.setContent(&fh_meta_file)) {
        qDebug() << "Cannot parse foil hole meta data: " << fh_meta_path;
        fh_meta_file.close();
        return result;
    }
    fh_meta_file.close();
    QString x=fh_meta_dom_document.elementsByTagName("a:x").at(0).toElement().text();
    data.insert("x",x);
    QString y=fh_meta_dom_document.elementsByTagName("a:y").at(0).toElement().text();
    data.insert("y",y);
    QString selected=fh_meta_dom_document.elementsByTagName("Selected").at(0).toElement().text();
    data.insert("selected",selected);
    data.setId(id);
    data.setParent(square_id);
    data.setTimestamp(last_modified);
    result.foil_holes.append(data);
    return result;
}

ParsedData readEPUGridSquareDM(const QFileInfo &info, const QString& project_dir, const QString &movie_dir)
{
    // read 2.7 foil hole metadata
    ParsedData result;
    QString meta_path=info.absoluteFilePath();
    QString square_id=info.baseName().remove(0,11);
    QFile meta_file(meta_path);
    QDateTime last_modified=QFileInfo(meta_file).lastModified();
    if (!meta_file.open(QIODevice::ReadOnly)){
        qDebug() << "Cannot open grid square meta data: " << meta_path;
        qDebug() << "Error was: " << meta_file.errorString() << " (" << meta_file.error() << ")";
        return result;
    }
    QDomDocument meta_dom_document;
    if (!meta_dom_document.setContent(&meta_file)) {
        qDebug() << "Cannot parse grid square meta data: " << meta_path;
        meta_file.close();
        return result;
    }
    meta_file.close();
    QDomNode target_location_array=meta_dom_document.elementsByTagName("a:m_serializationArray").at(0);
    QDomNode target_node = target_location_array.firstChild();
    while(!target_node.isNull()) {
        QString hole_id=target_node.toElement().elementsByTagName("b:key").at(0).toElement().text();
        Data data;
        data.setId(hole_id);
        data.setParent(square_id);
        QString x=target_node.toElement().elementsByTagName("c:x").at(0).toElement().text();
        if(x!=""){
            data.insert("x",x);
        }
        QString y=target_node.toElement().elementsByTagName("c:y").at(0).toElement().text();
        if(y!=""){
            data.insert("y",y);
        }
        QString selected=target_node.toElement().elementsByTagName("Selected").at(0).toElement().text();
        if(selected!=""){
            data.insert("selected",selected);
        }
        target_node = target_node.nextSibling();
        data.setTimestamp(last_modified);
        result.foil_holes.append(data);
    }
    return result;
}

DataFolderWatcher *createFlatEPUFolderWatcher(QObject *parent)
{
    DataFolderWatcher * watcher=new DataFolderWatcher(parent);
    FolderNode root;
    root.file_readers.append(QPair<QRegularExpression,FileReaderPtr>(QRegularExpression(".*\\.xml"),readEPUMicrographXML));
    watcher->setRootFolder(root);
    return watcher;
}

ParsedData readImage(const QFileInfo &info, const QString& project_dir, const QString& movie_dir)
{
    ParsedData result;
    QString json_path=QDir(info.absolutePath()).absoluteFilePath(info.completeBaseName()+".json");
    QFile json_file(json_path);
    if (!json_file.open(QIODevice::ReadOnly)) {
        qDebug() << "Couldn't open file: " << json_path;
        return result;
    }
    QByteArray data = json_file.readAll();
    QJsonDocument json_doc(QJsonDocument::fromJson(data));
    result.micrographs.append(QJsonObject(json_doc.object()));
    return result;


}

DataFolderWatcher *createFlatImageFolderWatcher(const QString &pattern, QObject *parent)
{
    DataFolderWatcher * watcher=new DataFolderWatcher(parent);
    FolderNode root;
    root.file_readers.append(QPair<QRegularExpression,FileReaderPtr>(QRegularExpression(pattern),readImage));
    watcher->setRootFolder(root);
    return watcher;
}

