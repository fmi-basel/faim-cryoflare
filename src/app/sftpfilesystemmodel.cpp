#include "sftpfilesystemmodel.h"
#include <QIcon>

PathNode::~PathNode(){
    qDeleteAll(children);
}
int PathNode::row() const{
    if (parent){
        return parent->children.indexOf(const_cast<PathNode*>(this));
    }
    return 0;
}
QString PathNode::path() const{
    QStringList path;
    const PathNode* node=this;
    do{
        path.prepend(node->name);
        node=node->parent;
    }while (node);
    return path.join("/");
}
bool PathNode::hasChildren() const{
    switch (type) {
    case UNPOPULATED_DIRECTORY:
        return true;
    case DIRECTORY:
        return ! children.isEmpty();
    case FILE:
    default:
        return false;
    }
}
SFTPAttributeList PathNode::fetchDirs(SFTPSession &session) const{
    SFTPAttributeList retval;
    if(UNPOPULATED_DIRECTORY==type){
        SFTPDirList dirlist=session.listDir(path());
        if(dirlist.ok){
            for (const auto& elem : dirlist.attribute_list){
                if(elem.name==".." || elem.name=="."){
                    continue;
                }
                retval.append(elem);
            }
        }
    }
    return retval;
}

void PathNode::insertDirs(const SFTPAttributeList &dirlist)
{
    type=DIRECTORY;
    for (const auto& elem :dirlist){
        children.append(new PathNode{elem.name,elem.type==SSH_FILEXFER_TYPE_DIRECTORY ?UNPOPULATED_DIRECTORY:FILE,QList<PathNode*>(),this });
    }
}

SFTPFileSystemModel::SFTPFileSystemModel(QObject *parent)
    : QAbstractItemModel{parent},
    session_(),
    root_(new PathNode{"/",PathNode::UNPOPULATED_DIRECTORY,QList<PathNode*>(),nullptr})
{}

bool SFTPFileSystemModel::connect(const QUrl &url)
{
    return session_.connect(url);
}

void SFTPFileSystemModel::disconnect()
{
    session_.disconnect();
}

QUrl SFTPFileSystemModel::getUrl() const
{
    return session_.getUrl();
}

int SFTPFileSystemModel::rowCount(const QModelIndex &parent) const
{
    PathNode *node;
    if (parent.column() > 0){
        return 0;
    }
    if (!parent.isValid()){
        node = root_.data();
    }else{
        node = static_cast<PathNode*>(parent.internalPointer());
    }
    return node->children.size();
}

int SFTPFileSystemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

QVariant SFTPFileSystemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()){
        return QVariant();
    }
    PathNode *node = static_cast<PathNode*>(index.internalPointer());
    switch (role) {
    case Qt::DisplayRole:
        if (index.column() == 1){
            return node->name;
        }
        break;
    case Qt::DecorationRole:
        if (index.column() == 0){
            switch (node->type) {
            case PathNode::DIRECTORY:
            case PathNode::UNPOPULATED_DIRECTORY:
                return QIcon(QLatin1String(":/icons/folder.png"));
            case PathNode::FILE:
            default:
                return QIcon(QLatin1String(":/icons/document.png"));
            }
        }
        break;
    case PathRole:
        if (index.column() == 1){
            return node->path();
        }
        break;
    case SortRole:
        if (index.column() == 1){
            //directories on top
            return QString("%1%2").arg(node->type==PathNode::FILE?1:0).arg(node->name);
        }
        break;
    default:
        break;
    }
    return QVariant();
}

Qt::ItemFlags SFTPFileSystemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()){
        return Qt::NoItemFlags;
    }
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;

}

bool SFTPFileSystemModel::hasChildren(const QModelIndex &parent) const
{
    PathNode *node;
    if (!parent.isValid()){
        node = root_.data();
    }else{
        node = static_cast<PathNode*>(parent.internalPointer());
    }
    return node->hasChildren();
}

QModelIndex SFTPFileSystemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)){
        return QModelIndex();
    }
    PathNode *parent_node;
    if (!parent.isValid()){
        parent_node = root_.data();
    } else {
        parent_node = static_cast<PathNode*>(parent.internalPointer());
    }
    if (!parent_node->children.isEmpty()){
        return createIndex(row, column, parent_node->children.at(row));
    }
    return QModelIndex();

}

QModelIndex SFTPFileSystemModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()){
        return QModelIndex();
    }
    PathNode *node = static_cast<PathNode*>(index.internalPointer());
    if (node->parent == root_.data()){
        return QModelIndex();
    }
    return createIndex(node->parent->row(), 0, node->parent);
}

QModelIndex SFTPFileSystemModel::pathToIndex(const QString &path)
{
    QModelIndex current_index;
    for (const auto& elem : path.split("/",Qt::SkipEmptyParts)){
        if(canFetchMore(current_index)){
            fetchMore(current_index);
        }
        bool found=false;
        for(int i=0; i<rowCount(current_index);++i){
            QModelIndex child_idx=index(i,1,current_index);
            if(data(child_idx,Qt::DisplayRole).toString()==elem){
                current_index=index(i,0,current_index);
                found=true;
                break;
            }
        }
        if(!found){
            return QModelIndex();
        }
    }
    return current_index;
}

void SFTPFileSystemModel::fetchMore(const QModelIndex &parent)
{
    PathNode *parent_node;
    if (!parent.isValid()){
        parent_node = root_.data();
    } else {
        parent_node = static_cast<PathNode*>(parent.internalPointer());
    }
    SFTPAttributeList children=parent_node->fetchDirs(session_);
    if(! children.isEmpty()){
        beginInsertRows(parent, 0, children.size() - 1);
        parent_node->insertDirs(children);
        endInsertRows();
    }
}

bool SFTPFileSystemModel::canFetchMore(const QModelIndex &parent) const
{
    PathNode *parent_node;
    if (!parent.isValid()){
        parent_node = root_.data();
    } else {
        parent_node = static_cast<PathNode*>(parent.internalPointer());
    }
    return parent_node->type==PathNode::UNPOPULATED_DIRECTORY;
}
