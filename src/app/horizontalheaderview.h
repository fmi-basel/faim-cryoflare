#ifndef HORIZONTALHEADERVIEW_H
#define HORIZONTALHEADERVIEW_H

#include <QHeaderView>

// QHeaderView with standard CTOR to allow promoting in designer
class HorizontalHeaderView : public QHeaderView
{
public:
    HorizontalHeaderView(QWidget * parent = 0);
};

#endif // HORIZONTALHEADERVIEW_H
