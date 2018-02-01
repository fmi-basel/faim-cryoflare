#include <QEnterEvent>
#include <QToolTip>
#include <QIcon>
#include "processindicator.h"

ProcessIndicator::ProcessIndicator(int gpu_id, QWidget *parent):
    QLabel(parent),
    timer_(),
    pos_(),
    process_id_(-1),
    display_string_()
{
    if(gpu_id>=0){
        setPixmap(QIcon(":/icons/gpu.png").pixmap(22,22));
        display_string_="Task: \%1 (\%2)\nGPU: "+QString("%1").arg(gpu_id)+"\n Image: \%3\nElapsed time: \%4";
    }else{
        setPixmap(QIcon(":/icons/cpu.png").pixmap(22,22));
        display_string_="Task: %1 (%2)\nImage: %3\nElapsed time: %4";
    }
    setDisabled(true);
    connect(&timer_,&QTimer::timeout,this,&ProcessIndicator::displayTooltip);
}

void ProcessIndicator::started(const QString &name, const QString &image, int process_id)
{
    elapsed_.start();
    task_=name;
    image_=image;
    process_id_=process_id;
    setEnabled(true);
}

void ProcessIndicator::finished()
{
    elapsed_.invalidate();
    setDisabled(true);
}

void ProcessIndicator::displayTooltip()
{
    QToolTip::showText(pos_, display_string_.arg(task_).arg(process_id_).arg(image_).arg(elapsed_.elapsed()/1000.0));
}

void ProcessIndicator::enterEvent(QEvent *event)
{
    if(!elapsed_.isValid()){
        return;
    }
    QEnterEvent* enter_event=dynamic_cast<QEnterEvent*>(event);
    pos_=mapToGlobal(enter_event->pos());
    displayTooltip();
    timer_.start(1000);
}

void ProcessIndicator::leaveEvent(QEvent *event)
{
    timer_.stop();
    QToolTip::hideText();
}
