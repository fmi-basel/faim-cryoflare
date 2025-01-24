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
#ifndef METADATASTORE_H
#define METADATASTORE_H

#include <QSet>
#include <QString>
#include <QList>
#include <QObject>
#include <QVector>
#include <QMap>
#include <QScopedPointer>
#include <QThread>
#include "dataptr.h"
#include "task.h"
#include "parallelexporter.h"

//fw decl
class DataFolderWatcher;
class TaskConfiguration;
class ParsedData;
class QUrl;

class PersistenDataWriter: public QObject
{
 Q_OBJECT
public slots:
    void writeData(Data data, const QString &basename);
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
    QSet<QString> sharedRawKeys() const;
    QList<QString> micrographIDs() const;
    QList<QString> selectedMicrographIDs() const;
    void setMicrographsExport(const QSet<QString>& ids,bool export_flag);
    void removeMicrographResults(const QString &id, const TaskDefinitionPtr& definition);
    void createReport(const QString& file_name, const QString & type);
    void exportMicrographs(const QUrl& destination, const QUrl& raw_export_path, const QStringList& output_keys, const QStringList& raw_keys, const QStringList& shared_keys, const QStringList& shared_raw_keys, bool duplicate_raw, bool create_reports=true);
    QString value(const QString& id, QString key) const;
    void stopUpdates();
    void resumeUpdates();
public slots:
    void updateMicrograph(const QString &id, const QMap<QString,QString>& new_data, const QMap<QString,QString>& raw_files=QMap<QString,QString>(), const QMap<QString,QString>& files=QMap<QString,QString>(), const QMap<QString,QString>& shared_files=QMap<QString,QString>(),const QMap<QString,QString>& shared_raw_files=QMap<QString,QString>());
    void updateData(const ParsedData& data, bool save=true);
    void start(const QString& project_dir, const QString &movie_dir);
    void stop();
signals:
    void newMicrograph(const QString & id);
    void newFoilhole(const QString & id);
    void newGridsquare(const QString & id);
    void micrographsUpdated(const QSet<QString> & ids, const QSet<QString> & keys);
    void foilholeUpdated(const QString & id, const QSet<QString> & keys);
    void gridsquareUpdated(const QString & id, const QSet<QString> & keys);
    void saveData(Data data,const QString& basename);
protected slots:
    void readPersistentData_();

protected:
    void saveMicrographData_(const QString& id, const QSet<QString> & keys);
    void saveFoilholeData_(const QString& id, const QSet<QString> & keys);
    void saveGridsquareData_(const QString& id, const QSet<QString> & keys);
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
    bool updates_stopped_;
    QSet<QString> queued_mic_ids_;
    QSet<QString> queued_mic_keys_;
};

#endif // METADATASTORE_H
