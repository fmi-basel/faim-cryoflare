#-------------------------------------------------
#
# Project created by QtCreator 2017-01-17T15:48:20
#
#-------------------------------------------------

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = StackGUI
TEMPLATE = app


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
    settings.cpp



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
    settings.h


FORMS    += \
    mainwindow.ui \
    settingsdialog.ui

#CONFIG += static
#static {
#    DEFINES += STATIC
#}
