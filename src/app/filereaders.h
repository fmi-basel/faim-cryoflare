//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2020 by the CryoFLARE Authors
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
#ifndef FILEREADERS_H
#define FILEREADERS_H

#include "datafolderwatcher.h"

DataFolderWatcher* createEPUFolderWatcher(QObject *parent=nullptr);
DataFolderWatcher* createFlatEPUFolderWatcher(QObject *parent=nullptr);
DataFolderWatcher *createFlatImageFolderWatcher(const QString & pattern,QObject *parent=nullptr);

ParsedData readEPUMicrographXML(const QFileInfo& info, const QString& project_dir, const QString& movie_dir);
ParsedData readEPUGridSquareXML(const QFileInfo& info, const QString& project_dir, const QString& movie_dir);
ParsedData readEPUTargetLocationDM(const QFileInfo& info, const QString& project_dir, const QString& movie_dir);
ParsedData readEPUGridSquareDM(const QFileInfo& info, const QString& project_dir, const QString& movie_dir);

ParsedData readImage(const QFileInfo& info, const QString& project_dir, const QString& movie_dir);


#endif // FILEREADERS_H
