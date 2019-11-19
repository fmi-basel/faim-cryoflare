//------------------------------------------------------------------------------
//
// Author: Andreas Schenk
// Friedrich Miescher Institute, Basel, Switzerland
//
// This file is part of CryoFLARE
//
// Copyright (C) 2017-2018 by the CryoFLARE Authors
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
// along with CryoFLARE.  If not, see <http://www.gnu.org/licenses/>.
//
//------------------------------------------------------------------------------

#include <sys/mman.h>
#include "mainwindow.h"
#include "micrographprocessor.h"
#include <QApplication>
#include <QStyleFactory>
#include <QHash>
#include <QTimer>
#include <QDebug>
#include <QtPlugin>
#include <QSharedPointer>
#include  "settings.h"
#include "filelocker.h"
#include "metadatastore.h"
#include "epudatasource.h"
#include "flatfolderdatasource.h"
#include "task.h"


Q_IMPORT_PLUGIN(MRCIOPlugin)

void crash_message_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtInfoMsg:
        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        __asm("int3");
        abort();
        break;
    }
}

DataSourceBase * createDataSource(const QString& mode, const QString& pattern){
    if(mode=="EPU"){
        return new EPUDataSource;
    }else if(mode=="flat_EPU"){
        return new FlatFolderDataSource(pattern,true);
    }else if(mode=="json"){
        return new FlatFolderDataSource(pattern,false);
    }else{
        return new EPUDataSource;
    }
}
int main(int argc, char* argv[])
{
    //qInstallMessageHandler(crash_message_handler);
    if(-1==mlockall(MCL_CURRENT|MCL_FUTURE)){
        //qWarning() << "failed to lock virtual address space into RAM";
    }
    QScopedPointer<QApplication> app(new QApplication(argc, argv));
    app.data()->setStyle(QStyleFactory::create("fusion"));
    QCoreApplication::setOrganizationName("Friedrich Miescher Institute");
    QCoreApplication::setOrganizationDomain("fmi.ch");
    QCoreApplication::setApplicationName("CryoFLARE");

    Settings settings;
    QDir cryoflare_dir(CRYOFLARE_DIRECTORY);
    if(! cryoflare_dir.exists()){
        QDir(".").mkdir(CRYOFLARE_DIRECTORY);
    }
    QDir gridsquare_dir(CRYOFLARE_DIRECTORY+"/"+CRYOFLARE_GRIDSQUARES_DIRECTORY);
    if(! gridsquare_dir.exists()){
        QDir(CRYOFLARE_DIRECTORY).mkdir(CRYOFLARE_GRIDSQUARES_DIRECTORY);
    }
    QDir foilhole_dir(CRYOFLARE_DIRECTORY+"/"+CRYOFLARE_FOILHOLES_DIRECTORY);
    if(! foilhole_dir.exists()){
        QDir(CRYOFLARE_DIRECTORY).mkdir(CRYOFLARE_FOILHOLES_DIRECTORY);
    }

    if(!settings.loadFromFile(CRYOFLARE_INI)){
        if(!settings.loadFromFile(".stack_gui.ini")){
            qWarning() << "No settings found in local directory. Using global settings.";
            settings.loadFromQSettings(QStringList() << "avg_source_dir" << "stack_source_dir");
            settings.saveToFile(CRYOFLARE_INI);
        }
    }
    FileLocker file_locker(CRYOFLARE_INI);
    if(!file_locker.tryLock()){
        int owner=file_locker.getLockOwner();
        if(owner==-1){
            qWarning() << "Can't get lock on "<< CRYOFLARE_INI << ". The directory might already be used by a different process. Please use a differnt directory or stop the other process first.";
        }else{
            qWarning() << "Directory is already used by process: " << owner << ". Please use a differnt directory or stop the other process first." ;
        }
        qWarning() << "Ignore (y/N):";
        int c=getchar();
        if(c!='y' && c!='Y'){
            return 1;
        }
    }
    QScopedPointer<TaskConfiguration> task_configuration(new TaskConfiguration);
    QScopedPointer<MetaDataStore> meta_data_store(new MetaDataStore(task_configuration.data()));
    //QObject::connect(app.data(),&QCoreApplication::aboutToQuit,meta_data_store.data(),&MetaDataStore::deleteLater);

    QScopedPointer<DataSourceBase> data_source(createDataSource(settings.value("import").toString(),settings.value("import_image_pattern").toString()));
    QObject::connect(data_source.data(),&DataSourceBase::newMicrograph,meta_data_store.data(),&MetaDataStore::addMicrograph);
    QObject::connect(data_source.data(),&DataSourceBase::newGridsquare,meta_data_store.data(),&MetaDataStore::addGridsquare);
    QObject::connect(data_source.data(),&DataSourceBase::newFoilhole,meta_data_store.data(),&MetaDataStore::addFoilhole);

    QScopedPointer<MicrographProcessor> processor(new MicrographProcessor(meta_data_store.data(),data_source.data(),task_configuration.data()));

    MainWindow w(meta_data_store.data(),processor.data(),task_configuration.data());
    QObject::connect(&w,&MainWindow::settingsChanged,task_configuration.data(),&TaskConfiguration::updateConfiguration);
    w.init();
    w.show();
    return app->exec();
}
