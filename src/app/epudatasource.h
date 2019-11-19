#ifndef EPUDATASOURCE_H
#define EPUDATASOURCE_H

#include <QScopedPointer>
#include <QDir>
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
    void parseGridSquares_(const QDir &dir);
    void parseFoilHoles_(const QDir &dir);
    void parseMicrographs_(const QDir &dir);
    QScopedPointer<FileSystemWatcher> watcher_;
    QString epu_project_dir_;
    QString movie_dir_;
    QStringList xml_files_;
    QHash<QString,Data> grid_square_data_;
    QHash<QString,Data> foil_hole_data_;

};

#endif // EPUDATASOURCE_H
