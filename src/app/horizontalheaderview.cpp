#include "horizontalheaderview.h"
#include <QDebug>
HorizontalHeaderView::HorizontalHeaderView( QWidget * parent):
    QHeaderView(Qt::Horizontal,parent),
    sibling_()
{
    setSectionResizeMode (QHeaderView::Fixed);
}

void HorizontalHeaderView::setSibling(QHeaderView *sibling)
{
    connect(sibling,&QHeaderView::sectionResized,this,&HorizontalHeaderView::resizeSection);
    connect(sibling,&QHeaderView::geometriesChanged,this,&HorizontalHeaderView::changeGeometry);
    sibling_=sibling;
}

void HorizontalHeaderView::changeGeometry()
{
    if(sibling_){
        qDebug() << sibling_->offset();
        setOffset(sibling_->offset());
    }
}
