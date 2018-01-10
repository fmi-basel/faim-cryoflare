TEMPLATE      = lib
CONFIG       += plugin static
QT           += widgets
HEADERS       = mrciohandler.h \
                mrcioplugin.h
SOURCES       = mrciohandler.cpp \
                mrcioplugin.cpp
TARGET        = $$qtLibraryTarget(mrcio)

