#include "tablesummarymodel.h"
#include <iostream>

TableSummaryModel::TableSummaryModel(ImageTableModel *model,QObject * parent):
    QAbstractTableModel(parent),
    model_(model)
{
    connect(model_,&ImageTableModel::dataChanged,this,&TableSummaryModel::sourceDataChanged);
}

QVariant TableSummaryModel::data(const QModelIndex &index, int role) const
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
                for(unsigned int i=0;i<model_->rowCount();++i){
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
                for(unsigned int i=0;i<model_->rowCount();++i){
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

int TableSummaryModel::rowCount(const QModelIndex &parent) const
{
    return 0;
}

int TableSummaryModel::columnCount(const QModelIndex &parent) const
{
    return model_->columnCount(parent);
}

void TableSummaryModel::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    emit headerDataChanged(Qt::Horizontal,topLeft.column(),bottomRight.column());
}
