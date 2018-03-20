//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFlare
//
// Copyright (C) 2017-2018 by the CryoFlare Authors
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3.0 of the License.
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License
// along with CryoFlare.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------------

#include <sys/mman.h>
#include "mainwindow.h"
#include "imageprocessor.h"
#include <QApplication>
#include <QStyleFactory>
#include <QHash>
#include <QTimer>
#include <QDebug>
#include <QtPlugin>
#include  "settings.h"
#include "filelocker.h"

QCoreApplication* createApplication(int &argc, char *argv[])
{
    for (int i = 1; i < argc; ++i)
        if (!qstrcmp(argv[i], "-no-gui"))
            return new QCoreApplication(argc, argv);
    return new QApplication(argc, argv);
}

Q_IMPORT_PLUGIN(MRCIOPlugin)

int main(int argc, char* argv[])
{
    if(-1==mlockall(MCL_CURRENT|MCL_FUTURE)){
        //qWarning() << "failed to lock virtual address space into RAM";
    }
    QScopedPointer<QCoreApplication> app(createApplication(argc, argv));
    QCoreApplication::setOrganizationName("Friedrich Miescher Institute");
    QCoreApplication::setOrganizationDomain("fmi.ch");
    QCoreApplication::setApplicationName("StackGUI");

    Settings settings;
    if(!settings.loadFromFile(".stack_gui.ini")){
        qWarning() << "No settings found in local directory. Using global settings.";
        settings.loadFromQSettings(QStringList() << "avg_source_dir" << "stack_source_dir");
        settings.saveToFile(".stack_gui.ini");
    }
    FileLocker file_locker(".stack_gui.ini");
    if(!file_locker.tryLock()){
        qWarning() << "Directory is already used by process: " << file_locker.getLockOwner() << ". Please use a differnt directory or stop the other process first." ;
        qWarning() << "Ignore (y/N):";
        int c=getchar();
        if(c!='y' && c!='Y'){
            return 1;
        }
    }
    ImageProcessor processor;
    if (qobject_cast<QApplication *>(app.data())) {
         // start GUI version...
        qobject_cast<QApplication *>(app.data())->setStyle(QStyleFactory::create("fusion"));
        MainWindow w;
        QObject::connect(&w, SIGNAL(startStop(bool)), &processor, SLOT(startStop(bool)));
        QObject::connect(&w,SIGNAL(exportImages(QString,QStringList)),&processor,SLOT(exportImages(QString,QStringList)));
        QObject::connect(&processor, SIGNAL(newImage(DataPtr)), &w, SLOT(addImage(DataPtr)));
        QObject::connect(&processor, SIGNAL(dataChanged(DataPtr)), &w, SLOT(onDataChanged(DataPtr)));
        QObject::connect(&w,SIGNAL(settingsChanged()),&processor,SLOT(loadSettings()));
        QObject::connect(&processor,SIGNAL(queueCountChanged(int,int)),&w,SLOT(updateQueueCounts(int,int)));
        QObject::connect(&processor,SIGNAL(processCreated(ProcessWrapper*,int)),&w,SLOT(createProcessIndicator(ProcessWrapper*,int)));
        QObject::connect(&processor,SIGNAL(processesDeleted()),&w,SLOT(deleteProcessIndicators()));
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
                    settings.setValue("avg_source_dir",app->arguments()[i]);
                    path_count=1;
                    break;
                case 1:
                    settings.setValue("stack_source_dir",app->arguments()[i]);
                    path_count=2;
                    break;
                }
            }
        }
        QTimer::singleShot(0, &processor, SLOT(startStop()));
        return app->exec();
     }
}
