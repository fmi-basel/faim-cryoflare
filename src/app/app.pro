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

QT       += core gui xml charts printsupport
greaterThan(QT_MAJOR_VERSION, 5): QT += widgets

TARGET = cryoflare
TEMPLATE = app
PRE_TARGETDEPS += ../mrcio/libmrcio.a

GIT_VERSION = $$system(git --git-dir $$PWD/../../.git --work-tree $$PWD/../.. describe --always --tags)
GIT_VERSION ~= s/-/"."
GIT_VERSION ~= s/g/""

DEFINES += GIT_VERSION=\\\"$$GIT_VERSION\\\"

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
    imagetablesortfilterproxymodel.cpp



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
    imagetablesortfilterproxymodel.h


FORMS    += \
    mainwindow.ui \
    settingsdialog.ui \
    aboutdialog.ui

LIBS     += ../mrcio/libmrcio.a 
CONFIG += static
static {
    DEFINES += STATIC
}

RESOURCES += \
    app.qrc

