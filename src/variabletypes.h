#ifndef VARIABLETYPES_H
#define VARIABLETYPES_H

#include <QStringList>

enum VariableType {
    String,
    Float,
    Int,
    Image
};

const static QStringList VariableTypeName(QStringList() << "String" << "Float" << "Int" << "Image");

#endif // VARIABLETYPES_H
