TARGET = limereport


TEMPLATE = lib
CONFIG += static_build

contains(CONFIG, static_build){
    CONFIG += staticlib
}

!contains(CONFIG, staticlib){
    CONFIG += lib
    CONFIG += dll
}

CONFIG += create_prl
CONFIG += link_prl


DEFINES += LIMEREPORT_EXPORTS

contains(CONFIG, staticlib){
    DEFINES += HAVE_STATIC_BUILD
    DEFINES -= LIMEREPORT_EXPORTS
}
QT += sql script
EXTRA_FILES += \
    $$PWD/lrglobal.cpp \
    $$PWD/lrglobal.h \
    $$PWD/lrdatasourcemanagerintf.h \
    $$PWD/lrreportengine.h \
    $$PWD/lrscriptenginemanagerintf.h \
    $$PWD/lrcallbackdatasourceintf.h \
    $$PWD/lrpreviewreportwidget.h

include(limereport.pri)



contains(CONFIG,zint){
    INCLUDEPATH += $$ZINT_PATH/backend $$ZINT_PATH/backend_qt4
    DEPENDPATH += $$ZINT_PATH/backend $$ZINT_PATH/backend_qt4


}

