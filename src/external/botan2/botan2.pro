#-------------------------------------------------
#
# Project created by QtCreator 2018-07-04T11:10:54
#
#-------------------------------------------------

QT       -= core gui

TARGET = botan2
TEMPLATE = lib
CONFIG += staticlib
CONFIG += warn_off

QMAKE_CXXFLAGS += -march=native
#QMAKE_CXXFLAGS += -march=skylake

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
	botan_all_aesni.cpp\
	botan_all_avx2.cpp\
	botan_all_bmi2.cpp\
	botan_all.cpp\
	botan_all_rdrand.cpp\
	botan_all_rdseed.cpp\
	botan_all_ssse3.cpp


HEADERS +=
unix {
    target.path = /usr/lib
    INSTALLS += target
}
