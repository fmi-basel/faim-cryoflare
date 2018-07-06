include(../common.pri)
QT += core gui

TARGET = LRDesigner
TEMPLATE = app

SOURCES += main.cpp

INCLUDEPATH += $$PWD/../limereport
DEPENDPATH  += ../limereport


macx{
    CONFIG  += app_bundle
}

unix:{
    CONFIG(debug, debug|release) {
        LIBS += -L../limereport  -llimereportd
    } else {
        LIBS += -L../limereport -llimereport
    }
    !contains(CONFIG, static_build){
		contains(CONFIG,zint){
                        LIBS += -L../3rdparty
                        CONFIG(debug, debug|release) {
                                LIBS += -lQtZintd
			} else {
                                LIBS += -lQtZint
			}
		}
	}
    DESTDIR = $$DEST_DIR
linux{
    #Link share lib to ../lib rpath
    QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN
    QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN/lib
    QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN/../lib
    QMAKE_LFLAGS_RPATH += #. .. ./libs
}
#    target.path = $${DEST_DIR}
#    INSTALLS = target
}

