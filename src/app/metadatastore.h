#ifndef MEATDATASTORE_H
#define MEATDATASTORE_H

#include <QObject>
#include <QVector>
#include <QScopedPointer>
#include "dataptr.h"

//fw decl
class DataSourceBase;

class MetaDataStore : public QObject
{
    Q_OBJECT
public:
    explicit MetaDataStore(QObject *parent = nullptr);
    ~MetaDataStore();
    void setDataSource(DataSourceBase *source);
    DataPtr at(int index) const;
    int size() const;
    int indexOf(const DataPtr& ptr) const;
    bool empty() const;
    void clear();
    QStringList rawFiles(const QStringList& image_list,const QStringList& key_list, bool finished_only=true) const;
    QStringList outputFiles(const QStringList& image_list,const QStringList& key_list, bool finished_only=true) const;
    QStringList sharedFiles(const QStringList& image_list,const QStringList& key_list, bool finished_only=true) const;
    QSet<QString> rawKeys() const;
    QSet<QString> outputKeys() const;
    QSet<QString> sharedKeys() const;
    QVector<DataPtr> images() const;
    QVector<DataPtr> selected() const;

public slots:
    void setProjectDir(const QString& epu_project_dir);
    void setMovieDir(const QString& movie_dir);
    void saveData(const DataPtr & ptr);
signals:
    void newImage(const DataPtr & ptr);
    void start();
    void stop();
protected slots:
    void addImage(const DataPtr & ptr);
protected:
    QScopedPointer<DataSourceBase> data_source_;
    QVector<DataPtr> data_;
};

#endif // MEATDATASTORE_H
