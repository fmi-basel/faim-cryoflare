//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFlare
//
// Copyright (C) 2017-2018 by the CryoFlare Authors
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
// along with CryoFlare.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------------

#ifndef RESULTVARIABLE_H
#define RESULTVARIABLE_H

#include <variabletypes.h>
#include <QVariant>
#include <QList>
#include <QtDebug>

class InputOutputVariable{
    enum SummaryType {
        NO_SUMMARY,
        SUM_SUMMARY,
        AVG_SUMMARY
    };
public:
    InputOutputVariable(const QString& key_="name", const QString &label_="variable",const VariableType& type_=String, bool in_column_=false,SummaryType summary_type_=NO_SUMMARY):
        key(key_),
        label(label_),
        type(type_),
        in_column(in_column_),
        summary_type(summary_type_)
    {}
    InputOutputVariable(const QVariant& v):
        key(),
        label(),
        type(),
        in_column(),
        summary_type(NO_SUMMARY)
    {
        if(v.type()==QVariant::List){
            QList<QVariant> list=v.toList();
            if(list.size()==4){
                key=list[0].toString();
                label=list[1].toString();
                type=static_cast<VariableType>(list[2].toInt());
                in_column=list[3].toBool();
                summary_type=static_cast<SummaryType>(list[4].toInt());
            }
        }
    }
    QVariant toQVariant() const
    {
        QList<QVariant> list;
        list << QVariant(key) << QVariant(label) << QVariant(type) << QVariant(in_column)<< QVariant(summary_type);
        return QVariant(list);
    }
    QString key;
    QString label;
    VariableType type;
    bool in_column;
    SummaryType summary_type;
};

#endif // RESULTVARIABLE_H
