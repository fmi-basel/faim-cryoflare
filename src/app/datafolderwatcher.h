#ifndef FOLDERWATCHER_H
#define FOLDERWATCHER_H

#include "dataptr.h"
#include "filesystemwatcher.h"
#include <QObject>
#include <QFileInfo>
#include <QRegularExpression>
#include <QFutureWatcher>


struct ParsedData{
    QList<Data> grid_squares;
    QList<Data> foil_holes;
    QList<Data> micrographs;
};
typedef ParsedData (*FileReaderPtr)(const QFileInfo& info);
typedef QPair<QRegularExpression,FileReaderPtr> ReaderPair;
struct FolderNode {
    QRegularExpression pattern;
    QList<FolderNode> children;
    QList<ReaderPair> file_readers;
};


class DataFolderWatcher : public QObject
{
    Q_OBJECT
public:
    explicit DataFolderWatcher(QObject *parent = nullptr);
    void setRootFolder(const FolderNode& node);

signals:
    void newDataAvailable(const ParsedData& data);
public slots:
    void setProjectDir(const QString& project_dir);
    void start();
    void stop();
protected slots:
    virtual void onDirChanged(const QString & path, QList<QFileInfo> changed_files);
    void fileReadFinished(QFutureWatcher<ParsedData> * watcher);
protected:
    FolderNode root_folder_;
    QScopedPointer<FileSystemWatcher> watcher_;
    QString project_dir_;
    QStringList watched_dirs_;
    QList<QFutureWatcher<ParsedData> *> future_watchers_;
};

#endif // FOLDERWATCHER_H
