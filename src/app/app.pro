#------------------------------------------------------------------------------
#
# Author: Andreas Schenk
# Friedrich Miescher Institute, Basel, Switzerland
#
# This file is part of CryoFlare
#
# Copyright (C) 2017-2018 by the CryoFlare Authors
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
# along with CryoFlare.  If not, see <http://www.gnu.org/licenses/>.
#
#------------------------------------------------------------------------------

QT       += core gui xml charts printsupport network script sql
greaterThan(QT_MAJOR_VERSION, 5): QT += widgets

TARGET = cryoflare
TEMPLATE = app
PRE_TARGETDEPS += ../mrcio/libmrcio.a
PRE_TARGETDEPS += ../external/botan2/libbotan2.a
PRE_TARGETDEPS += ../external/qssh/libqssh.a
PRE_TARGETDEPS += ../external/limereport/3rdparty/libQtZint.a
PRE_TARGETDEPS += ../external/limereport/limereport/liblimereport.a
#LIBS += -L$$OUT_PWD/../external/limereport/limereport/ -llimereport
#LIBS += -L$$OUT_PWD/../external/limereport/3rdparty/ -lQtZint
INCLUDEPATH += $$PWD/../external/limereport/limereport


GIT_VERSION = $$system(git --git-dir $$PWD/../../.git --work-tree $$PWD/../.. describe --always --tags)
GIT_VERSION ~= s/-/"."
GIT_VERSION ~= s/g/""

DEFINES += GIT_VERSION=\\\"$$GIT_VERSION\\\"

version_target.target = aboutdialog.hh
version_target.depends = FORCE
version_target.commands = if [ -e aboutdialog.o ];then rm aboutdialog.o 2> /dev/null;fi
QMAKE_EXTRA_TARGETS += version_target
PRE_TARGETDEPS += $$version_target.target


SOURCES += \
    main.cpp\
    mainwindow.cpp \
    filesystemwatcher.cpp \
    filesystemwatcherimpl.cpp \
    epuimageinfo.cpp \
    imageprocessor.cpp \
    processwrapper.cpp \
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
    sshauthenticationdialog.cpp \
    sshauthenticationstore.cpp \
    sftpurl.cpp \
    diskusagewidget.cpp \
    lastimagetimer.cpp \
    metadatastore.cpp \
    datasourcebase.cpp \
    epudatasource.cpp



HEADERS  += \
    mainwindow.h \
    filesystemwatcher.h \
    filesystemwatcherimpl.h \
    epuimageinfo.h \
    imageprocessor.h \
    processwrapper.h \
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
    sshauthenticationdialog.h \
    sshauthenticationstore.h \
    sftpurl.h \
    diskusagewidget.h \
    lastimagetimer.h \
    metadatastore.h \
    datasourcebase.h \
    epudatasource.h


FORMS    += \
    mainwindow.ui \
    settingsdialog.ui \
    aboutdialog.ui \
    scatterplotdialog.ui \
    exportdialog.ui \
    remotefiledialog.ui \
    exportprogressdialog.ui \
    sshauthenticationdialog.ui

LIBS += ../mrcio/libmrcio.a
LIBS += ../external/qssh/libqssh.a
LIBS += ../external/botan2/libbotan2.a
LIBS += ../external/limereport/limereport/liblimereport.a
LIBS += ../external/limereport/3rdparty/libQtZint.a
LIBS += -ldl
CONFIG += static
static {
    DEFINES += STATIC
}


RESOURCES += \
    app.qrc

DISTFILES +=


