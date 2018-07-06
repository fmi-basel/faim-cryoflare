/***************************************************************************
 *   This file is part of the Lime Report project                          *
 *   Copyright (C) 2015 by Alexander Arin                                  *
 *   arin_a@bk.ru                                                          *
 *                                                                         *
 **                   GNU General Public License Usage                    **
 *                                                                         *
 *   This library is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 **                  GNU Lesser General Public License                    **
 *                                                                         *
 *   This library is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library.                                      *
 *   If not, see <http://www.gnu.org/licenses/>.                           *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 ****************************************************************************/
#include "lrimageitem.h"
#include "lrdesignelementsfactory.h"
#include "lrglobal.h"
#include "lrdatasourcemanager.h"

namespace{

const QString xmlTag = "ImageItem";

LimeReport::BaseDesignIntf * createImageItem(QObject* owner, LimeReport::BaseDesignIntf*  parent){
    return new LimeReport::ImageItem(owner,parent);
}
bool VARIABLE_IS_NOT_USED registred = LimeReport::DesignElementsFactory::instance().registerCreator(
                     xmlTag, LimeReport::ItemAttribs(QObject::tr("Image Item"),"Item"), createImageItem
                 );
}

namespace LimeReport{

ImageItem::ImageItem(QObject* owner,QGraphicsItem* parent)
    :ItemDesignIntf(xmlTag,owner,parent),m_autoSize(false), m_scale(true), m_keepAspectRatio(true), m_center(true), m_format(Binary){}

BaseDesignIntf *ImageItem::createSameTypeItem(QObject *owner, QGraphicsItem *parent)
{
    return new ImageItem(owner,parent);
}

void ImageItem::updateItemSize(DataSourceManager* dataManager, RenderPass pass, int maxHeight)
{

   if (m_picture.isNull()){
       if (!m_datasource.isEmpty() && !m_field.isEmpty()){
           IDataSource* ds = dataManager->dataSource(m_datasource);
           if (ds) {
              QVariant data = ds->data(m_field);
              if (data.isValid()){
                  if (data.type()==QVariant::Image){
                    m_picture =  data.value<QImage>();
                  } else {
                      switch (m_format) {
                      default:
                      case Binary:
                          m_picture.loadFromData(data.toByteArray());
                          break;
                      case Hex:
                          m_picture.loadFromData(QByteArray::fromHex(data.toByteArray()));
                          break;
                      case Base64:
                          m_picture.loadFromData(QByteArray::fromBase64(data.toByteArray()));
                          break;
                      case Filename:
                          m_picture.load(data.toString());
                          break;
                      }
                  }

              }
           }
       } else if (!m_resourcePath.isEmpty()){
           m_picture = QImage(m_resourcePath);
       }
   }
   if (m_autoSize){
       setWidth(m_picture.width());
       setHeight(m_picture.height());
   }
   BaseDesignIntf::updateItemSize(dataManager, pass, maxHeight);
}

bool ImageItem::isNeedUpdateSize(RenderPass) const
{
    return m_picture.isNull() || m_autoSize;
}

QString ImageItem::resourcePath() const
{
    return m_resourcePath;
}

qreal ImageItem::minHeight() const{
    if (!m_picture.isNull() && autoSize())
    {
        return m_picture.height();
    } else {
        return 0;
    }
}

bool ImageItem::center() const
{
    return m_center;
}

void ImageItem::setCenter(bool center)
{
    if (m_center != center){
        m_center = center;
        update();
        notify("center",!center,center);
    }
}

bool ImageItem::keepAspectRatio() const
{
    return m_keepAspectRatio;
}

void ImageItem::setKeepAspectRatio(bool keepAspectRatio)
{
    if (m_keepAspectRatio != keepAspectRatio){
        m_keepAspectRatio = keepAspectRatio;
        update();
        notify("keepAspectRatio",!keepAspectRatio,keepAspectRatio);
    }
}

bool ImageItem::scale() const
{
    return m_scale;
}

void ImageItem::setScale(bool scale)
{
    if (m_scale != scale){
        m_scale = scale;
        update();
        notify("scale",!scale,scale);
    }
}
bool ImageItem::autoSize() const
{
    return m_autoSize;
}

void ImageItem::setAutoSize(bool autoSize)
{
    if (m_autoSize != autoSize){
        m_autoSize = autoSize;
        if (m_autoSize && !m_picture.isNull()){
            setWidth(image().width());
            setHeight(image().height());
            setPossibleResizeDirectionFlags(Fixed);
        } else {
            setPossibleResizeDirectionFlags(AllDirections);
        }
        update();
        notify("autoSize",!autoSize,autoSize);
    }
}

QString ImageItem::field() const
{
    return m_field;
}

void ImageItem::setField(const QString &field)
{
    if (m_field != field){
        QString oldValue = m_field;
        m_field = field;
        update();
        notify("field",oldValue,field);
    }
}

QString ImageItem::datasource() const
{
    return m_datasource;
}

void ImageItem::setDatasource(const QString &datasource)
{
    if (m_datasource != datasource){
        QString oldValue = m_datasource;
        m_datasource = datasource;
        update();
        notify("datasource",oldValue,datasource);
    }
}


void ImageItem::paint(QPainter *ppainter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    ppainter->save();
    if (isSelected()) ppainter->setOpacity(Const::SELECTION_OPACITY);
    else ppainter->setOpacity(qreal(opacity())/100);

    QPointF point = rect().topLeft();
    QImage img;

    if (m_scale && !image().isNull()){
        img = image().scaled(rect().width(), rect().height(), keepAspectRatio() ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    } else {
        img = image();
    }

    qreal shiftHeight = rect().height() - img.height();
    qreal shiftWidth = rect().width() - img.width();

    if (m_center){
        if (shiftHeight<0 || shiftWidth<0){
            qreal cutX = 0;
            qreal cutY = 0;
            qreal cutWidth = img.width();
            qreal cutHeigth = img.height();

            if (shiftWidth > 0){
                point.setX(point.x()+shiftWidth/2);
            } else {
                cutX = fabs(shiftWidth/2);
                cutWidth += shiftWidth;
            }

            if (shiftHeight > 0){
                point.setY(point.x()+shiftHeight/2);
            } else {
                cutY = fabs(shiftHeight/2);
                cutHeigth += shiftHeight;
            }

            img = img.copy(cutX,cutY,cutWidth,cutHeigth);
        } else {
            point.setX(point.x()+shiftWidth/2);
            point.setY(point.y()+shiftHeight/2);
        }
    }

    if (img.isNull() && itemMode()==DesignMode){
        QString text;
        ppainter->setFont(transformToSceneFont(QFont("Arial",10)));
        ppainter->setPen(Qt::black);
        if (!datasource().isEmpty() && !field().isEmpty())
            text = datasource()+"."+field();
        else text = tr("Image");
        ppainter->drawText(rect().adjusted(4,4,-4,-4), Qt::AlignCenter, text );
    } else {
        ppainter->drawImage(point,img);
    }
    ItemDesignIntf::paint(ppainter,option,widget);
    ppainter->restore();
}

void ImageItem::setImage(QImage value)
{
    if (m_picture!=value){
        QImage oldValue = m_picture;
        m_picture=value;
        if (m_autoSize){
            setWidth(m_picture.width());
            setHeight(m_picture.height());
        }
        update();
        notify("image",oldValue,value);
    }
}

ImageItem::Format ImageItem::format() const
{
    return m_format;
}

void ImageItem::setFormat(Format format)
{
    if (m_format!=format){
        Format oldValue = m_format;
        m_format=format;
        update();
        notify("format",oldValue,format);
    }
}

}
