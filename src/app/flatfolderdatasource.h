#ifndef FLATFOLDERDATASOURCE_H
#define FLATFOLDERDATASOURCE_H

#include <QFileInfo>
#include <QScopedPointer>
#include "datasourcebase.h"

//fw decl
class FileSystemWatcher;
class QTimer;

class FlatFolderDataSource : public DataSourceBase
{
    Q_OBJECT
public:
    FlatFolderDataSource(const QString& pattern, bool xml=true);
    virtual ~FlatFolderDataSource();
public slots:
    virtual void start();
    virtual void stop();
    virtual void setProjectDir(const QString& project_dir);
    virtual void setMovieDir(const QString& movie_dir);
protected slots:
    virtual void onDirChanged(const QString & path);
    virtual void checkForFileChanges();
protected:
    DataPtr readJson_(const QString & path);
    QScopedPointer<FileSystemWatcher> watcher_;
    QString project_dir_;
    QString movie_dir_;
    QStringList image_files_;
    QHash<QString,DataPtr> grid_square_data_;
    QString pattern_;
    bool xml_;
    QTimer* check_file_timer_;
    QHash<QString,QPair<QDateTime,qint64> > watched_files_;
};



#endif // FLATFOLDERDATASOURCE_H
