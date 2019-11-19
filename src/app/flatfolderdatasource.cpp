#include <QTimer>
#include <QDebug>
#include <QJsonDocument>
#include <QDateTime>
#include <QDir>
#include "readepuxml.h"
#include "filesystemwatcher.h"
#include "flatfolderdatasource.h"

FlatFolderDataSource::FlatFolderDataSource(const QString &pattern, bool xml):
    DataSourceBase(),
    watcher_(new FileSystemWatcher()),
    project_dir_(),
    movie_dir_(),
    image_files_(),
    grid_square_data_(),
    pattern_(pattern),
    xml_(xml),
    check_file_timer_(new QTimer(this)),
    watched_files_()
{
    connect(watcher_.data(),&FileSystemWatcher::directoryChanged,this,&FlatFolderDataSource::onDirChanged);
    connect(check_file_timer_,&QTimer::timeout,this,&FlatFolderDataSource::checkForFileChanges);
}

FlatFolderDataSource::~FlatFolderDataSource()
{

}

void FlatFolderDataSource::start()
{
    watcher_->addPath(project_dir_);
}

void FlatFolderDataSource::stop()
{
    watcher_->removeAllPaths();
}

void FlatFolderDataSource::setProjectDir(const QString &project_dir)
{
    project_dir_=project_dir;
}

void FlatFolderDataSource::setMovieDir(const QString &movie_dir)
{
    movie_dir_=movie_dir;
}

void FlatFolderDataSource::onDirChanged(const QString &path)
{
    QDir dir(path);
    QFileInfoList image_files=dir.entryInfoList(QStringList(pattern_));
    for(int i=0;i<image_files.size();++i){
        QString image_path=image_files.at(i).absoluteFilePath();
        if(!image_files_.contains(image_path)){
            image_files_.append(image_path);
            watched_files_.insert(image_path,QPair<QDateTime,qint64>(image_files.at(i).lastModified(),image_files.at(i).size()));
            check_file_timer_->start(2000);
        }
    }

}

void FlatFolderDataSource::checkForFileChanges()
{
    foreach(QString image_path,watched_files_.keys()){
        QFileInfo file_info(image_path);
        if(file_info.lastModified()==watched_files_.value(image_path).first && file_info.size()==watched_files_.value(image_path).second){
            watched_files_.remove(image_path);
            Data data;
            if(xml_){
                QString xml_path=QDir(file_info.absolutePath()).absoluteFilePath(file_info.completeBaseName()+".xml");
                data=readEPUXML(xml_path);
            }else{
                QString json_path=QDir(file_info.absolutePath()).absoluteFilePath(file_info.completeBaseName()+".json");
                data=readJson_(json_path);
            }
            if(!data.empty()){
                data.insert("destination_path",QDir::currentPath());
                data.insert("stack_source_path",movie_dir_);
                data.insert("avg_source_path",project_dir_);
                QStringList stack_frames;
                if(QString("BM-Falcon")==data.value("camera").toString()){
                    data.insert("stack_frames",QString("%1/%2_frames.mrc").arg(movie_dir_).arg(data.value("name").toString()));
                }else if(QString("EF-CCD")==data.value("camera").toString()){
                    for(int i=1;i<=data.value("num_frames").toInt();++i){
                        stack_frames.append(QString("%1/%2-*-%3.???").arg(movie_dir_).arg(data.value("name").toString()).arg(i,4,10,QChar('0')));
                    }
                    data.insert("stack_frames",stack_frames.join(" "));
                }
                emit newMicrograph(data);
            }
        }else{
            watched_files_.insert(image_path,QPair<QDateTime,qint64>(file_info.lastModified(),file_info.size()));
        }
    }
    if(watched_files_.empty()){
        check_file_timer_->stop();
    }
}

Data FlatFolderDataSource::readJson_(const QString &path)
{
    QFile json_file(path);
    if (!json_file.open(QIODevice::ReadOnly)) {
        qDebug() << "Couldn't open file: " << path;
        return Data();
    }
    QByteArray data = json_file.readAll();
    QJsonDocument json_doc(QJsonDocument::fromJson(data));
    return QJsonObject(json_doc.object());
}

