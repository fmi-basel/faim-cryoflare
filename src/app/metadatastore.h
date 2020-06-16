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
#ifndef MEATDATASTORE_H
#define MEATDATASTORE_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QScopedPointer>
#include <QThread>
#include "dataptr.h"
#include "task.h"
#include "sftpurl.h"
#include "parallelexporter.h"

//fw decl
class DataFolderWatcher;
class TaskConfiguration;
class ParsedData;

class PersistenDataWriter: public QObject
{
 Q_OBJECT
public slots:
    void writeData(QJsonObject data, const QString &basename);
};


class MetaDataStore : public QObject
{
    Q_OBJECT
public:
    explicit MetaDataStore(TaskConfiguration* task_confguration, QObject *parent = nullptr);
    ~MetaDataStore();
    Data at(int index) const;
    Data micrograph(const QString& id) const;
    Data gridsquare(const QString& id) const;
    QMap<QString,Data> gridsquares() const;
    QMap<QString,Data> foilholes() const;
    Data gridsquareAt(int index) const;
    Data foilhole(const QString& id) const;
    bool hasGridsquare(const QString& id) const;
    bool hasFoilhole(const QString& id) const;
    int gridsquareCount() const;
    int foilholeCount() const;
    int size() const;
    int indexOf(const QString& id) const;
    bool empty() const;
    void clear();
    QSet<QString> rawKeys() const;
    QSet<QString> outputKeys() const;
    QSet<QString> sharedKeys() const;
    QList<QString> micrographIDs() const;
    QList<QString> selectedMicrographIDs() const;
    void setMicrographExport(const QString& id,bool export_flag);
    void removeMicrographResults(const QString &id, const TaskDefinitionPtr& definition);
    void createReport(const QString& file_name, const QString & type);
    void exportMicrographs(const SftpUrl& destination, const SftpUrl& raw_export_path, const QStringList& output_keys, const QStringList& raw_keys, const QStringList& shared_keys, bool duplicate_raw);
    QString value(const QString& id, QString key) const;
public slots:
    void updateMicrograph(const QString &id, const QMap<QString,QString>& new_data, const QMap<QString,QString>& raw_files=QMap<QString,QString>(), const QMap<QString,QString>& files=QMap<QString,QString>(), const QMap<QString,QString>& shared_files=QMap<QString,QString>());
    void updateFoilhole(const Data & data);
    void updateData(const ParsedData& data, bool save=true);
    void start(const QString& project_dir);
    void stop();
signals:
    void newMicrograph(const QString & id);
    void newFoilhole(const QString & id);
    void newGridsquare(const QString & id);
    void micrographUpdated(const QString & id);
    void foilholeUpdated(const QString & id);
    void gridsquareUpdated(const QString & id);
    void saveData(QJsonObject data,const QString& basename);
protected slots:
    void readPersistenData();
protected:
    void saveMicrographData_(const QString& id);
    void saveFoilholeData_(const QString& id);
    void saveGridsquareData_(const QString& id);
    QList<Data> readPersistentDataHelper_(const QString & path);
    void exportFinished_();
    void startNextExport_();
    DataFolderWatcher * createFolderWatcher_(const QString& mode="EPU", const QString& pattern="");
    TaskConfiguration* task_configuration_;
    DataFolderWatcher * data_folder_watcher_;
    QMap<QString,Data> micrographs_;
    QMap<QString,Data> foil_holes_;
    QMap<QString,Data> grid_squares_;
    QThread worker_;
    QQueue<ParallelExporter*> exporters_;
    ParallelExporter* current_exporter_;

};

#endif // MEATDATASTORE_H
