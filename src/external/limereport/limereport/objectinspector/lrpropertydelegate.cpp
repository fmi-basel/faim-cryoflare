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
#include "lrpropertydelegate.h"
#include "lrobjectitemmodel.h"
#include "lrobjectinspectorwidget.h"
#include <QPainter>
#include <QLineEdit>
#include <QApplication>
#include "lrglobal.h"

LimeReport::PropertyDelegate::PropertyDelegate(QObject *parent)
    :QItemDelegate(parent), m_objectInspector(NULL), m_editingItem(0), m_isEditing(false)
{}

void LimeReport::PropertyDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) return;

    LimeReport::ObjectPropItem *node = static_cast<LimeReport::ObjectPropItem*>(index.internalPointer());
    if (node){
         if (!node->isHaveValue()){
            if (index.column()==0) {
                StyleOptionViewItem cellOpt = option;
                QTreeView const *tree = dynamic_cast<const QTreeView*>(cellOpt.widget);
                QStyleOptionViewItem primitiveOpt = cellOpt;
                primitiveOpt.rect.setWidth(tree->indentation());
                painter->save();
                painter->setPen(option.palette.color(QPalette::HighlightedText));
                painter->setBackground(QBrush(option.palette.color(QPalette::Highlight)));
                drawBackground(painter,option,index);
                cellOpt.widget->style()->drawPrimitive(QStyle::PE_IndicatorBranch,&primitiveOpt,painter);
                cellOpt.rect.adjust(primitiveOpt.rect.width(),0,0,0);
                cellOpt.font.setBold(true);
                cellOpt.palette.setColor(QPalette::Text,cellOpt.palette.color(QPalette::BrightText));
                drawDisplay(painter,cellOpt,cellOpt.rect,LimeReport::extractClassName(node->propertyName()));
                painter->restore();
            }
         } else
         {
             if (index.column()==0){
                 QPointF start(
                             option.rect.x()+option.rect.width()-1,
                             option.rect.y()
                             );
                 QPointF end(
                             option.rect.x()+option.rect.width()-1,
                             option.rect.y()+option.rect.height()
                             );
                 painter->save();
                 QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &option));
                 painter->setPen(color);
                 painter->drawLine(start,end);
                 painter->restore();
             }

             StyleOptionViewItem so = option;
             if ((node->isValueReadonly())&&(!node->isHaveChildren())) {
                 so.palette.setColor(QPalette::Text,so.palette.color(QPalette::Dark));
             }

             QColor backgroundColor = (so.features & StyleOptionViewItem::Alternate) ?
                         so.palette.alternateBase().color() :
                         so.palette.base().color();

             qreal luma = 0.2126 * backgroundColor.red() +
                          0.7152 * backgroundColor.green() +
                          0.0722 * backgroundColor.blue();

             if (luma<128)
                so.palette.setColor(QPalette::Text,Qt::white);
             else
                so.palette.setColor(QPalette::Text,Qt::black);

             drawBackground(painter,option,index);
             if (!node->paint(painter,so,index))
                QItemDelegate::paint(painter, so, index);
         }
    }
}

QSize LimeReport::PropertyDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &/*index*/) const
{
    QSize size=option.rect.size();
    size.setHeight(option.fontMetrics.height()+
                   QApplication::style()->pixelMetric(QStyle::PM_ButtonMargin)
#ifdef Q_OS_MAC
                   +QApplication::style()->pixelMetric(QStyle::PM_FocusFrameVMargin)
#endif
                  +2);
    return size;
}

QWidget * LimeReport::PropertyDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    m_editingItem=static_cast<LimeReport::ObjectPropItem*>(index.internalPointer());
    connect(m_editingItem,SIGNAL(destroyed(QObject*)), this, SLOT(slotItemDeleted(QObject*)));
    QWidget *editor=m_editingItem->createProperyEditor(parent);
    if (editor){
        m_isEditing = true;
        editor->setMaximumHeight(option.rect.height()-1);
        editor->setGeometry(option.rect);
        if (editor->metaObject()->indexOfSignal("editingFinished()")!=-1)
            connect(editor,SIGNAL(editingFinished()),this,SLOT(commitAndCloseEditor()));
        connect(editor,SIGNAL(destroyed()),this,SLOT(slotEditorDeleted()));
    }
    return editor;
}

void LimeReport::PropertyDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (m_editingItem) m_editingItem->setPropertyEditorData(editor,index);
}

void LimeReport::PropertyDelegate::commitAndCloseEditor()
{
    QWidget *editor = qobject_cast<QWidget*>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}

void LimeReport::PropertyDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (m_editingItem) m_editingItem->setModelData(editor,model,index);
}

void LimeReport::PropertyDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (m_editingItem) m_editingItem->updateEditorGeometry(editor,option,index);
}

void LimeReport::PropertyDelegate::setObjectInspector(ObjectInspectorWidget* objectInspector)
{
    m_objectInspector=objectInspector;
}

void LimeReport::PropertyDelegate::slotEditorDeleted()
{
    m_isEditing=false;
}

void LimeReport::PropertyDelegate::slotItemDeleted(QObject *item)
{
    if (item == m_editingItem) m_editingItem = 0;
}

LimeReport::ObjectPropItem* LimeReport::PropertyDelegate::editingItem()
{
    return m_editingItem;
}

