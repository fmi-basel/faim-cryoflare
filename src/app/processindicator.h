#ifndef PROCESSINDICATOR_H
#define PROCESSINDICATOR_H
#include <QElapsedTimer>
#include <QLabel>
#include <QString>
#include <QPixmap>
#include <QTimer>

class ProcessIndicator : public QLabel
{
    Q_OBJECT
public:
    ProcessIndicator(int gpu_id, QWidget * parent = 0);
public slots:
    void started(const QString & name,  const QString& image, int process_id);
    void finished();
    void displayTooltip();
protected:
    virtual void enterEvent(QEvent *event);
    virtual void leaveEvent(QEvent *event);

private:
    QElapsedTimer elapsed_;
    QString task_;
    QString image_;
    QTimer timer_;
    QPoint pos_;
    int process_id_;
    QString display_string_;
};

#endif // PROCESSINDICATOR_H
