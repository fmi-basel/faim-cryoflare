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
