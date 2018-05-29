#ifndef HORIZONTALHEADERVIEW_H
#define HORIZONTALHEADERVIEW_H

#include <QHeaderView>
#include <QTableView>

// QHeaderView with standard CTOR to allow promoting in designer
class HorizontalHeaderView : public QHeaderView
{
public:
    HorizontalHeaderView(QWidget * parent = 0);
    void setSibling(QTableView* sibling);
public slots:
    void siblingSectionResized(int logicalIndex, int oldSize, int newSize);
    void scrolled(int logical_index);
protected:
    QTableView* sibling_;
};

#endif // HORIZONTALHEADERVIEW_H
