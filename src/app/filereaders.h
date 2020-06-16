#ifndef FILEREADERS_H
#define FILEREADERS_H

#include "datafolderwatcher.h"

DataFolderWatcher* createEPUFolderWatcher(QObject *parent=nullptr);
DataFolderWatcher* createFlatEPUFolderWatcher(QObject *parent=nullptr);
DataFolderWatcher *createFlatImageFolderWatcher(const QString & pattern,QObject *parent=nullptr);

ParsedData readEPUMicrographXML(const QFileInfo& info);
ParsedData readEPUGridSquareXML(const QFileInfo& info);
ParsedData readEPUTargetLocationDM(const QFileInfo& info);
ParsedData readEPUGridSquareDM(const QFileInfo& info);

ParsedData readImage(const QFileInfo& info);


#endif // FILEREADERS_H
