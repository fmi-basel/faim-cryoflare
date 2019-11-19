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
class DataSourceBase;
class TaskConfiguration;

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
public slots:
    void addMicrograph(Data data);
    void addFoilhole(const Data & data);
    void addGridsquare(const Data & data);
    void updateMicrograph(const QString &id, const QMap<QString,QString>& new_data, const QMap<QString,QString>& raw_files=QMap<QString,QString>(), const QMap<QString,QString>& files=QMap<QString,QString>(), const QMap<QString,QString>& shared_files=QMap<QString,QString>());
    void updateFoilhole(const Data & data);
    void updateGridsquare(const Data & data);
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
    void readPersistentDataHelper_(const QString & path, QMap<QString,Data> & storage, void (MetaDataStore::*sig)(const QString&));
    void exportFinished_();
    void startNextExport_();
    TaskConfiguration* task_configuration_;
    QMap<QString,Data> micrographs_;
    QMap<QString,Data> foil_holes_;
    QMap<QString,Data> grid_squares_;
    QThread worker_;
    QQueue<ParallelExporter*> exporters_;
    ParallelExporter* current_exporter_;

};

#endif // MEATDATASTORE_H
