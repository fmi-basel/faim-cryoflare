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
#include "tablesummarymodel.h"
#include <iostream>

TableSummaryModel::TableSummaryModel(ImageTableModel *model,QObject * parent):
    QAbstractTableModel(parent),
    model_(model),
    update_timer_()
{
    connect(model_,&ImageTableModel::dataChanged,this,&TableSummaryModel::sourceDataChanged);
    update_timer_.setSingleShot(true);
    connect(&update_timer_, &QTimer::timeout, this, &TableSummaryModel::update);
}

QVariant TableSummaryModel::data(const QModelIndex &/*index*/, int /*role*/) const
{
    return QVariant();
}

QVariant TableSummaryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role==Qt::DisplayRole){
        if(orientation==Qt::Horizontal){
            InputOutputVariable::SummaryType summary_type;
            if(section==0){
                summary_type=InputOutputVariable::SUM_SUMMARY;
            }else{
                summary_type=static_cast<InputOutputVariable::SummaryType>(model_->headerData(section,orientation,ImageTableModel::SummaryRole).toInt());
            }
            if(summary_type==InputOutputVariable::SUM_SUMMARY){
                float sum=0;
                for(int i=0;i<model_->rowCount();++i){
                    QVariant val=model_->data(model_->index(i,section),ImageTableModel::SortRole);
                    if(val.canConvert<float>() && val.toString()!=QString("")){
                        float fval=val.toFloat();
                        sum+=fval;
                    }
                }

                return QString("S:%1").arg(sum);
            }else if(summary_type==InputOutputVariable::AVG_SUMMARY){
                float sum=0;
                int count=0;
                for(int i=0;i<model_->rowCount();++i){
                    QVariant val=model_->data(model_->index(i,section),ImageTableModel::SortRole);
                    if(val.canConvert<float>() && val.toString()!=QString("")){
                        float fval=val.toFloat();
                        sum+=fval;
                        count+=1;
                    }
                }
                return QString("A:%1").arg(sum/std::max(1,count));
            }else{
                return "";
            }
        }
    }     return model_->headerData(section,orientation,role);
}

int TableSummaryModel::rowCount(const QModelIndex &/*parent*/) const
{
    return 0;
}

int TableSummaryModel::columnCount(const QModelIndex &parent) const
{
    return model_->columnCount(parent);
}

void TableSummaryModel::sourceDataChanged(const QModelIndex &/*topLeft*/, const QModelIndex &/*bottomRight*/, const QVector<int> &/*roles*/)
{
    update_timer_.start();
}

void TableSummaryModel::update()
{
    emit headerDataChanged(Qt::Horizontal,0,columnCount()-1);
}
