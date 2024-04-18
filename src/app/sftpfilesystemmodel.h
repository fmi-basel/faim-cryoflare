#ifndef SFTPFILESYSTEMMODEL_H
#define SFTPFILESYSTEMMODEL_H

#include "sftpsession.h"
#include <QAbstractItemModel>

struct PathNode{
    ~PathNode();
    int row() const;
    QString path() const;
    bool hasChildren() const;
    SFTPAttributeList fetchDirs(SFTPSession &session) const;
    void insertDirs(const SFTPAttributeList& dirlist);
    enum NodeType{ UNPOPULATED_DIRECTORY,DIRECTORY,FILE};
    QString name;
    NodeType type;
    QList<PathNode*> children;
    PathNode * parent;

};


class SFTPFileSystemModel : public QAbstractItemModel
{
public:
    enum roles{
        PathRole=Qt::UserRole,
        SortRole
    };
    explicit SFTPFileSystemModel(QObject *parent = nullptr);
    bool connect(const QUrl& url);
    void disconnect();
    QUrl getUrl() const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    virtual QModelIndex pathToIndex(const QString & path);
    virtual void fetchMore(const QModelIndex &parent);
    virtual bool canFetchMore(const QModelIndex &parent) const;
protected:
    SFTPSession session_;
    QScopedPointer<PathNode> root_;
};

#endif // SFTPFILESYSTEMMODEL_H
