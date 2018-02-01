#-------------------------------------------------
#
# Project created by QtCreator 2017-01-17T15:48:20
#
#-------------------------------------------------

QT       += core gui xml charts printsupport
greaterThan(QT_MAJOR_VERSION, 5): QT += widgets

TARGET = StackGUI
TEMPLATE = app
PRE_TARGETDEPS += ../mrcio/libmrcio.a

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
    aboutdialog.cpp



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
    aboutdialog.h


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
