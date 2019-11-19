#include "imagetablemodel.h"
#include "linearchartview.h"

#include <QLineSeries>
#include <QPushButton>

LinearChartView::LinearChartView(QWidget *parent):
    ChartView (parent)
{

}

void LinearChartView::drawSeries_()
{
    QList<QPointF> all_data;
    QList<QList<QPointF> > selected_data;
    double last_idx=0;
    QList<QPointF> current_series;
    for(int i=0;i<model_->rowCount();++i){
        QVariant val=model_->data(model_->index(i,active_column_),ImageTableModel::SortRole);
        Data data=model_->image(i);
        QString export_val=data.value("export").toString("true");
        bool export_flag=export_val.compare("true", Qt::CaseInsensitive) == 0 || export_val==QString("1") ;
        if(val.canConvert<float>() && val.toString()!=QString("")){
            qreal fval=val.toDouble();
            QPointF p(i,fval);
            if(export_flag){
                if(i>last_idx+1 && current_series.count()>0){
                    selected_data.append(current_series);
                    current_series = QList<QPointF>();
                }
                current_series << p;
                last_idx=i;
            }
            all_data.append(p);
        }
    }
    if(current_series.count()>0){
        selected_data.append(current_series);
    }

    QtCharts::QLineSeries *full_series = new QtCharts::QLineSeries();
    full_series->setColor(color_);
    full_series->setPointsVisible();
    full_series->append(all_data);
    chart()->addSeries(full_series);

    foreach( QList<QPointF> line, selected_data){
        QtCharts::QLineSeries *series = new QtCharts::QLineSeries();
        series->setColor(selected_color_);
        series->setPointsVisible();
        series->append(line);
        chart()->addSeries(series);
        //connect(series,&QtCharts::QLineSeries::hovered,this,&MainWindow::displayLinearChartDetails);
    }
}

void LinearChartView::deselectData_(float start, float end, bool invert)
{
    int istart=std::max(0,static_cast<int>(start));
    int iend=std::min(model_->rowCount()-1,static_cast<int>(end));
    if(invert){
        for(int i=istart;i<=iend;++i){
            model_->setData(model_->index(i,0),Qt::Unchecked,Qt::CheckStateRole);
        }
    }else{
        for(int i=0;i<istart;++i){
            model_->setData(model_->index(i,0),Qt::Unchecked,Qt::CheckStateRole);
        }
        for(int i=iend+1;i<model_->rowCount();++i){
            model_->setData(model_->index(i,0),Qt::Unchecked,Qt::CheckStateRole);
        }
    }
}
