#------------------------------------------------------------------------------
#
# Author: Andreas Schenk
# Friedrich Miescher Institute, Basel, Switzerland
#
# This file is part of CryoFLARE
#
# Copyright (C) 2017-2020 by the CryoFLARE Authors
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 3.0 of the License.
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License
# along with CryoFLARE.  If not, see <http://www.gnu.org/licenses/>.
#
#------------------------------------------------------------------------------

QT       += core gui xml charts printsupport network script sql concurrent
greaterThan(QT_MAJOR_VERSION, 5): QT += widgets

TARGET = cryoflare
TEMPLATE = app
PRE_TARGETDEPS += ../mrcio/libmrcio.a
PRE_TARGETDEPS += ../external/botan2/libbotan2.a
PRE_TARGETDEPS += ../external/qssh/libqssh.a
CONFIG(debug, debug|release) {
    PRE_TARGETDEPS += ../external/limereport/3rdparty/libQtZintd.a
    PRE_TARGETDEPS += ../external/limereport/limereport/liblimereportd.a
} else {
    PRE_TARGETDEPS += ../external/limereport/3rdparty/libQtZint.a
    PRE_TARGETDEPS += ../external/limereport/limereport/liblimereport.a
}
INCLUDEPATH += $$PWD/../external/limereport/limereport



GITVERSION = $$OUT_PWD/version.h
version_target.target =  $$GITVERSION
version_target.commands = '$$PWD/git_version.sh \"$$PWD\" $$GITVERSION'
version_target.depends = FORCE
PRE_TARGETDEPS += $$GITVERSION
QMAKE_EXTRA_TARGETS += version_target

SOURCES += \
    datafolderwatcher.cpp \
    filereaders.cpp \
    main.cpp\
    mainwindow.cpp \
    filesystemwatcher.cpp \
    filesystemwatcherimpl.cpp \
    processwrapper.cpp \
    sftpfilesystemmodel.cpp \
    sftpsession.cpp \
    sshsession.cpp \
    task.cpp \
    imagetablemodel.cpp \
    tasktreewidgetitem.cpp \
    pathedit.cpp \
    imagetableview.cpp \
    parallelexporter.cpp \
    settingsdialog.cpp \
    settings.cpp \
    filelocker.cpp \ 
    processindicator.cpp \
    chartview.cpp \
    aboutdialog.cpp \
    positionchart.cpp \
    positionchartview.cpp \
    imagetablesortfilterproxymodel.cpp \
    scatterplotdialog.cpp \
    horizontalheaderview.cpp \
    tablesummarymodel.cpp \
    exportdialog.cpp \
    remotefiledialog.cpp \
    remotepathedit.cpp \
    exportprogressdialog.cpp \
    diskusagewidget.cpp \
    lastimagetimer.cpp \
    metadatastore.cpp \
    datasourcebase.cpp \
    collection.cpp \
    collectionstartingcondition.cpp \
    collectiondefinition.cpp \
    jobmanager.cpp \
    job.cpp \
    flatfolderdatasource.cpp \
    readepuxml.cpp \
    priorityqueue.cpp \
    gridsquaretablemodel.cpp \
    imageviewer.cpp \
    micrographsform.cpp \
    gridsquareform.cpp \
    datachartform.cpp \
    linearchartview.cpp \
    histogramchartview.cpp \
    histogram.cpp \
    phaseplatechart.cpp \
    micrographprocessor.cpp \
    processqueue.cpp \
    gradient.cpp \
    scatterchartview.cpp



HEADERS  += \
    datafolderwatcher.h \
    filereaders.h \
    mainwindow.h \
    filesystemwatcher.h \
    filesystemwatcherimpl.h \
    processwrapper.h \
    sftpfilesystemmodel.h \
    sftpsession.h \
    sshsession.h \
    task.h \
    imagetablemodel.h \
    pathedit.h \
    variabletypes.h \
    inputoutputvariable.h \
    imagetableview.h \
    parallelexporter.h \
    settingsdialog.h \
    settings.h \
    filelocker.h \ 
    processindicator.h \
    chartview.h \
    aboutdialog.h \
    positionchart.h \
    positionchartview.h \
    imagetablesortfilterproxymodel.h \
    scatterplotdialog.h \
    horizontalheaderview.h \
    tablesummarymodel.h \
    exportdialog.h \
    remotefiledialog.h \
    remotepathedit.h \
    exportprogressdialog.h \
    diskusagewidget.h \
    lastimagetimer.h \
    metadatastore.h \
    datasourcebase.h \
    collection.h \
    collectionstartingcondition.h \
    collectiondefinition.h \
    jobmanager.h \
    job.h \
    flatfolderdatasource.h \
    readepuxml.h \
    priorityqueue.h \
    gridsquaretablemodel.h \
    imageviewer.h \
    micrographsform.h \
    gridsquareform.h \
    datachartform.h \
    linearchartview.h \
    histogramchartview.h \
    histogram.h \
    phaseplatechart.h \
    micrographprocessor.h \
    processqueue.h \
    gradient.h \
    scatterchartview.h


FORMS    += \
    mainwindow.ui \
    settingsdialog.ui \
    aboutdialog.ui \
    scatterplotdialog.ui \
    exportdialog.ui \
    remotefiledialog.ui \
    exportprogressdialog.ui \
    micrographsform.ui \
    gridsquareform.ui \
    datachartform.ui

LIBS += ../mrcio/libmrcio.a
LIBS += ../external/qssh/libqssh.a
LIBS += ../external/botan2/libbotan2.a
CONFIG(debug, debug|release) {
    LIBS += ../external/limereport/limereport/liblimereportd.a
    LIBS += ../external/limereport/3rdparty/libQtZintd.a
} else {
    LIBS += ../external/limereport/limereport/liblimereport.a
    LIBS += ../external/limereport/3rdparty/libQtZint.a
}
LIBS += -ldl
LIBS += -lssh
CONFIG += static
static {
    DEFINES += STATIC
}


RESOURCES += \
    app.qrc

DISTFILES += \
    license_header.txt \
    git_versio.sh


# debug flags for address sanitizer
#QMAKE_LFLAGS_DEBUG += -fsanitize=address -fsanitize=undefined  -static-libasan -static-libubsan
#QMAKE_CXXFLAGS_DEBUG += -fsanitize=address -fsanitize=undefined  -static-libasan -static-libubsan
# debug flags for profiler
#QMAKE_CXXFLAGS_RELEASE += -pg
#QMAKE_LFLAGS_RELEASE += -pg
