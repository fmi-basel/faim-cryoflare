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
#include "lrbarcodeitem.h"
#include "lrdesignelementsfactory.h"
#include "qzint.h"
#include "lrglobal.h"

namespace{

const QString xmlTag = "BarcodeItem";

LimeReport::BaseDesignIntf * createBarcodeItem(QObject* owner, LimeReport::BaseDesignIntf*  parent){
    return new LimeReport::BarcodeItem(owner,parent);
}
bool VARIABLE_IS_NOT_USED registred = LimeReport::DesignElementsFactory::instance().registerCreator(xmlTag, LimeReport::ItemAttribs(QObject::tr("Barcode Item"),"Item"), createBarcodeItem);

}

namespace LimeReport{

BarcodeItem::BarcodeItem(QObject* owner,QGraphicsItem* parent)
    : ContentItemDesignIntf(xmlTag,owner,parent),m_designTestValue("1"), m_barcodeType(CODE128),
      m_foregroundColor(Qt::black), m_backgroundColor(Qt::white), m_whitespace(10), m_angle(Angle0),
      m_barcodeWidth(0), m_securityLevel(0), m_pdf417CodeWords(928), m_inputMode(UNICODE_INPUT_MODE)
{}

BarcodeItem::~BarcodeItem()
{}

BaseDesignIntf *BarcodeItem::createSameTypeItem(QObject *owner, QGraphicsItem *parent)
{
    return new BarcodeItem(owner,parent);
}

void BarcodeItem::paint(QPainter *ppainter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    ppainter->save();
    Zint::QZint bc;
    if (itemMode() & DesignMode) bc.setText(m_designTestValue);
    else bc.setText(m_content);
    bc.setInputMode(m_inputMode);
    bc.setSymbol(m_barcodeType);
    bc.setWhitespace(m_whitespace);
    bc.setFgColor(m_foregroundColor);
    bc.setBgColor(m_backgroundColor);
    bc.setWidth(m_barcodeWidth);
    bc.setSecurityLevel(m_securityLevel);
    bc.setPdf417CodeWords(m_pdf417CodeWords);

    if (isSelected()) ppainter->setOpacity(Const::SELECTION_OPACITY);

    QRectF bcRect;

    switch (m_angle) {
    case Angle0:
        bcRect = rect();
        break;
    case Angle90:
        ppainter->translate(width(),0);
        ppainter->rotate(90);
        bcRect = QRectF(0,0,height(),width());
        break;
    case Angle180:
        bcRect = rect();
        ppainter->translate(width(),height());
        ppainter->rotate(180);
        break;
    case Angle270:
        ppainter->translate(0,height());
        ppainter->rotate(270);
        bcRect = QRectF(0,0,height(),width());
        break;
    }

    bc.render(*ppainter,bcRect,Zint::QZint::KeepAspectRatio);
    ppainter->restore();
    ItemDesignIntf::paint(ppainter,option,widget);
}

void BarcodeItem::setContent(const QString &content)
{
    if (m_content!=content){
        QString oldValue = m_content;
        m_content=content;
        update();
        notify("content",oldValue,m_content);
    }
}

void BarcodeItem::setBarcodeType(BarcodeItem::BarcodeType value)
{
    if (m_barcodeType!=value){
        BarcodeType oldValue = m_barcodeType;
        m_barcodeType = value;
        update();
        notify("barcodeType",oldValue,value);
    }
}

void BarcodeItem::setDesignTestValue(QString value)
{
    if (m_designTestValue!=value){
        QString oldValue = m_designTestValue;
        m_designTestValue=value;
        update();
        notify("testValue",oldValue,value);
    }
}

void BarcodeItem::setForegroundColor(QColor value)
{
    if (m_foregroundColor != value){
        QColor oldValue = m_foregroundColor;
        m_foregroundColor=value;
        update();
        notify("foregroundColor",oldValue,value);
    }
}

void BarcodeItem::setBackgroundColor(QColor value)
{
    if (m_backgroundColor != value){
        QColor oldValue = m_backgroundColor;
        m_backgroundColor=value;
        update();
        notify("backgroundColor",oldValue,value);
    }
}

void BarcodeItem::setWhitespace(int value)
{
    if (m_whitespace != value){
        int oldValue = m_whitespace;
        m_whitespace = value;
        update();
        notify("whitespace",oldValue,value);
    }
}

BarcodeItem::AngleType BarcodeItem::angle() const
{
    return m_angle;
}

void BarcodeItem::setAngle(const AngleType &angle)
{
    if (m_angle!=angle){
        AngleType oldValue = m_angle;
        m_angle = angle;
        if (!isLoading()){
            update();
            notify("angle",oldValue,angle);
        }
    }
}

int BarcodeItem::barcodeWidth() const
{
    return m_barcodeWidth;
}

void BarcodeItem::setBarcodeWidth(int barcodeWidth)
{
    if (m_barcodeWidth != barcodeWidth){
        int oldValue = m_barcodeWidth;
        m_barcodeWidth = barcodeWidth;
        if (!isLoading()){
            update();
            notify("barcodeWidth",oldValue,m_barcodeWidth);
        }
    }
}

int BarcodeItem::securityLevel() const
{
    return m_securityLevel;
}

void BarcodeItem::setSecurityLevel(int securityLevel)
{
    if (m_securityLevel != securityLevel){
        int oldValue = m_securityLevel;
        m_securityLevel = securityLevel;
        if (!isLoading()){
            update();
            notify("securityLevel",oldValue,m_securityLevel);
        }
    }
}

int BarcodeItem::pdf417CodeWords() const
{
    return m_pdf417CodeWords;
}

void BarcodeItem::setPdf417CodeWords(int pdf417CodeWords)
{
    if (m_pdf417CodeWords != pdf417CodeWords){
        int oldValue = m_pdf417CodeWords;
        m_pdf417CodeWords = pdf417CodeWords;
        if (!isLoading()){
            update();
            notify("pdf417CodeWords",oldValue,m_pdf417CodeWords);
        }
    }
}

BarcodeItem::InputMode BarcodeItem::inputMode() const
{
    return m_inputMode;
}

void BarcodeItem::setInputMode(const InputMode &inputMode)
{
    if (m_inputMode != inputMode){
        InputMode oldValue = m_inputMode;
        m_inputMode = inputMode;
        if (!isLoading()){
            update();
            notify("inputMode",oldValue,inputMode);
        }
    }
}

void BarcodeItem::updateItemSize(DataSourceManager* dataManager, RenderPass pass, int maxHeight)
{
    switch(pass){
    case FirstPass:
        setContent(expandUserVariables(content(),pass,NoEscapeSymbols, dataManager));
        setContent(expandDataFields(content(), NoEscapeSymbols, dataManager));
        break;
    default:;
    }
    BaseDesignIntf::updateItemSize(dataManager, pass, maxHeight);
}

bool BarcodeItem::isNeedUpdateSize(RenderPass pass) const
{return  (pass==FirstPass)?true:false;}

}
