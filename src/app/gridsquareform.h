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
#ifndef GRIDSQUAREFORM_H
#define GRIDSQUAREFORM_H

#include "gridsquaretablemodel.h"
#include "gradient.h"
#include <QWidget>

//fw decl
class TaskConfiguration;

namespace Ui {
class GridsquareForm;
}

class GridsquareForm : public QWidget
{
    Q_OBJECT

public:
    explicit GridsquareForm(QWidget *parent = nullptr);
    ~GridsquareForm();
    void init(MetaDataStore* meta_data_store, TaskConfiguration* task_config);
public slots:
    void loadGridsquare();
    void updateMarkers();
    void updateResultLabels();
    void deselectMicrographs(QStringList& ids,bool invert);

private:
    Ui::GridsquareForm *ui;
    MetaDataStore* meta_data_store_;
    GridsquareTableModel *gridsquare_model_;
    TaskConfiguration* task_configuration_;
    Gradient gradient_;
};

#endif // GRIDSQUAREFORM_H
