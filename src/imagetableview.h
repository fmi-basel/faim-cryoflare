#ifndef IMAGETABLEVIEW_H
#define IMAGETABLEVIEW_H

#include <QTableView>

class ImageTableView : public QTableView
{
    Q_OBJECT
public:
    explicit ImageTableView(QWidget *parent = 0);

signals:

public slots:
    void selectEverything();
    void unselectEverything();
    void selectAbove();
    void unselectAbove();
    void selectBelow();
    void unselectBelow();
    void invertSelection();

protected:
    QAction *select_all_;
    QAction *unselect_all_;
    QAction *select_above_;
    QAction *unselect_above_;
    QAction *select_below_;
    QAction *unselect_below_;
    QAction *invert_selection_;

};

#endif // IMAGETABLEVIEW_H
