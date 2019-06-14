#include "horizontalheaderview.h"
#include <QDebug>
#include <QScrollBar>
HorizontalHeaderView::HorizontalHeaderView( QWidget * parent):
    QHeaderView(Qt::Horizontal,parent),
    sibling_()
{
    setSectionResizeMode (QHeaderView::Fixed);
}

void HorizontalHeaderView::setSibling(QTableView *sibling)
{
    connect(sibling->horizontalHeader(),&QHeaderView::sectionResized,this,&HorizontalHeaderView::siblingSectionResized);
    connect(sibling->horizontalScrollBar(),&QAbstractSlider::valueChanged,this,&HorizontalHeaderView::scrolled);
    sibling_=sibling;
}

void HorizontalHeaderView::siblingSectionResized(int logicalIndex, int /*oldSize*/, int newSize)
{
    reset();
    QHeaderView::resizeSection(logicalIndex,newSize);
    scrolled(sibling_->horizontalScrollBar()->value());
}

void HorizontalHeaderView::scrolled(int /*logical_index*/)
{
    setOffset(-1-sibling_->verticalHeader()->geometry().width()-sibling_->horizontalHeader()->sectionViewportPosition(0));
}

