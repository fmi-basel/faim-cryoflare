#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <QDomDocument>
#include "readepuxml.h"

Data readEPUXML(const QString &path)
{
    Data result;
    QDir xml_dir(path);
    QString name=xml_dir.dirName();
    name.truncate(name.lastIndexOf('.'));


    result.insert("xml_file",path);
    result.insert("name",name);
    QStringList splitted_name=name.split("_");
    if(splitted_name.size()>=7){
        //original EPU file not from flat folder
        result.insert("hole_id",splitted_name.at(1));
        result.insert("template_id",splitted_name.at(3));
        result.insert("acquisition_id",splitted_name.at(4));
    }

    QDomDocument dom_document;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)){
        qDebug() << "Cannot open XML file: " << path;
        return Data();
    }
    if (!dom_document.setContent(&file)) {
        qDebug() << "Cannot parse XML file: " << path;
        file.close();
        return Data();
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
    result.insert("apix_x",QString("%1").arg(pixel_size_values.at(0).toElement().text().toFloat()*1e10/result.value("super_resolution_factor").toDouble()));
    result.insert("apix_y",QString("%1").arg(pixel_size_values.at(1).toElement().text().toFloat()*1e10/result.value("super_resolution_factor").toDouble()));


    result.insert("acceleration_voltage",QString("%1").arg(dom_document.elementsByTagName("AccelerationVoltage").at(0).toElement().text().toFloat()/1000.0));
    QDomNode nominal_magnification=dom_document.elementsByTagName("NominalMagnification").at(0);
    result.insert("nominal_magnification",nominal_magnification.toElement().text());
    QDomNode datetime=dom_document.elementsByTagName("acquisitionDateTime").at(0);
    QString timestamp=datetime.toElement().text();
    QDateTime time=QDateTime::fromString(timestamp,Qt::ISODate);
    result.insert("timestamp",time.toString("yyyy-MM-dd hh:mm:ss"));
    result.insert("short_name",time.toString("yyyyMMdd_hhmmss"));
    result.insert("id",time.toString("yyyyMMdd_hhmmss"));
    QDomNode position=dom_document.elementsByTagName("Position").at(0);
    result.insert("X",position.toElement().elementsByTagName("X").at(0).toElement().text());
    result.insert("Y",position.toElement().elementsByTagName("Y").at(0).toElement().text());
    result.insert("Z",position.toElement().elementsByTagName("Z").at(0).toElement().text());
    result.insert("A",position.toElement().elementsByTagName("A").at(0).toElement().text());
    result.insert("B",position.toElement().elementsByTagName("B").at(0).toElement().text());
    QDomNode image_shift=dom_document.elementsByTagName("ImageShift").at(0);
    result.insert("image_shift_x",image_shift.toElement().elementsByTagName("a:_x").at(0).toElement().text());
    result.insert("image_shift_y",image_shift.toElement().elementsByTagName("a:_y").at(0).toElement().text());

    return result;

}
