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
#ifndef COLLECTION_H
#define COLLECTION_H

#include "dataptr.h"
#include <QObject>
#include <QVector>

//fw decl
class CollectionStartingCondition;
class CollectionDefinition;

class Collection : public QObject
{
    Q_OBJECT
public:
    explicit Collection(QObject *parent = nullptr);

signals:

public slots:
    void checkStartingCondition();
    void start();
protected:
    void assembleData_();
    void submitJob();
    CollectionStartingCondition* starting_condition_;
    CollectionDefinition* definition_;
    QVector<Data> data_;
};

#endif // COLLECTION_H
