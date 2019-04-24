#ifndef EPUDATASOURCE_H
#define EPUDATASOURCE_H

#include <QScopedPointer>
#include "datasourcebase.h"

//fw decl
class FileSystemWatcher;

class EPUDataSource : public DataSourceBase
{
    Q_OBJECT
public:
    EPUDataSource();
    virtual ~EPUDataSource();
public slots:
    virtual void start();
    virtual void stop();
    virtual void setProjectDir(const QString& epu_project_dir);
    virtual void setMovieDir(const QString& movie_dir);
protected slots:
    virtual void onDirChanged(const QString & path);
protected:
    QScopedPointer<FileSystemWatcher> watcher_;
    QString epu_project_dir_;
    QString movie_dir_;
    QStringList xml_files_;
    QHash<QString,DataPtr> grid_square_data_;

};

#endif // EPUDATASOURCE_H
