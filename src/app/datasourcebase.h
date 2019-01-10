#ifndef DATASOURCEBASE_H
#define DATASOURCEBASE_H

#include <QObject>
#include "dataptr.h"

class DataSourceBase : public QObject
{
    Q_OBJECT
public:
    explicit DataSourceBase(QObject *parent = nullptr);
    virtual ~DataSourceBase();

signals:
    void newImage(const DataPtr& ptr);

public slots:
    virtual void start()=0;
    virtual void stop()=0;
    virtual void setProjectDir(const QString& epu_project_dir)=0;
    virtual void setMovieDir(const QString& movie_dir)=0;
};

#endif // DATASOURCEBASE_H