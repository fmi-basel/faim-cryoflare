#ifndef PROCESSQUEUE_H
#define PROCESSQUEUE_H

#include <QObject>

class ProcessQueue : public QObject
{
    Q_OBJECT
public:
    explicit ProcessQueue(QObject *parent = nullptr);

signals:

public slots:
};

#endif // PROCESSQUEUE_H