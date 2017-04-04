#ifndef RESULTVARIABLE_H
#define RESULTVARIABLE_H

#include <variabletypes.h>
#include <QVariant>
#include <QList>
#include <QtDebug>

class InputOutputVariable{
public:
    InputOutputVariable(const QString& key_="name", const QString &label_="variable",const VariableType& type_=String, bool in_column_=false):
        key(key_),
        label(label_),
        type(type_),
        in_column(in_column_)
    {}
    InputOutputVariable(const QVariant& v):
        key(),
        label(),
        type(),
        in_column()
    {
        if(v.type()==QVariant::List){
            QList<QVariant> list=v.toList();
            if(list.size()==4){
                key=list[0].toString();
                label=list[1].toString();
                type=static_cast<VariableType>(list[2].toInt());
                in_column=list[3].toBool();
            }
        }
    }
    QVariant toQVariant() const
    {
        QList<QVariant> list;
        list << QVariant(key) << QVariant(label) << QVariant(type) << QVariant(in_column);
        return QVariant(list);
    }
    QString key;
    QString label;
    VariableType type;
    bool in_column;
};

#endif // RESULTVARIABLE_H
