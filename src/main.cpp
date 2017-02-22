#include "mainwindow.h"
#include "imageprocessor.h"
#include <QApplication>
#include <QHash>
#include <QTimer>
#include <QDebug>


QCoreApplication* createApplication(int &argc, char *argv[])
{
    for (int i = 1; i < argc; ++i)
        if (!qstrcmp(argv[i], "-no-gui"))
            return new QCoreApplication(argc, argv);
    return new QApplication(argc, argv);
}

int main(int argc, char* argv[])
{
    QScopedPointer<QCoreApplication> app(createApplication(argc, argv));
    QCoreApplication::setOrganizationName("Friedrich Miescher Institute");
    QCoreApplication::setOrganizationDomain("fmi.ch");
    QCoreApplication::setApplicationName("StackGUI");
    ImageProcessor processor;
    if (qobject_cast<QApplication *>(app.data())) {
         // start GUI version...
        MainWindow w;
        QObject::connect(&w, SIGNAL(avgSourceDirChanged(QString)), &processor, SLOT(setAvgSourcePath(const QString&)));
        QObject::connect(&w, SIGNAL(stackSourceDirChanged(QString)), &processor, SLOT(setStackSourcePath(const QString&)));
        QObject::connect(&w, SIGNAL(destinationDirChanged(QString)), &processor, SLOT(setDestinationPath(const QString&)));
        QObject::connect(&w, SIGNAL(startStop(bool)), &processor, SLOT(startStop(bool)));
        QObject::connect(&processor, SIGNAL(newImage(DataPtr)), &w, SLOT(addImage(DataPtr)));
        QObject::connect(&processor, SIGNAL(tasksChanged(TaskPtr)), &w, SLOT(onTasksChanged(TaskPtr)));
        QObject::connect(&processor, SIGNAL(dataChanged(DataPtr)), &w, SLOT(onDataChanged(DataPtr)));
        processor.init();
        w.init();
        w.show();
        //next line within if to avoid MainWindow going out of scope
        return app->exec();
     } else {
         // start non-GUI version...
        int path_count=0;
        for (int i = 1; i < app->arguments().size(); ++i){
            if (qstrcmp(argv[i], "-no-gui")){
                switch(path_count){
                case 0:
                    processor.setAvgSourcePath(app->arguments()[i]);
                    path_count=1;
                    break;
                case 1:
                    processor.setStackSourcePath(app->arguments()[i]);
                    path_count=2;
                    break;
                case 2:
                    processor.setDestinationPath(app->arguments()[i]);
                    path_count=3;
                    break;
                }
            }
        }
        processor.init();
        QTimer::singleShot(0, &processor, SLOT(startStop()));
        return app->exec();
     }
}
