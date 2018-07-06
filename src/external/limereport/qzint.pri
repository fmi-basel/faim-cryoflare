DEFINES +=  _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS ZINT_VERSION=\\\"$$ZINT_VERSION\\\" NO_PNG

!contains(DEFINES, NO_PNG) {
    LIBS += -lpng
}

INCLUDEPATH += \
    $$ZINT_PATH/backend \
    $$ZINT_PATH/backend_qt4

HEADERS +=  $$ZINT_PATH/backend/aztec.h \
            $$ZINT_PATH/backend/code1.h \
            $$ZINT_PATH/backend/code49.h \
            $$ZINT_PATH/backend/common.h \
            $$ZINT_PATH/backend/composite.h \
            $$ZINT_PATH/backend/dmatrix.h \
            $$ZINT_PATH/backend/font.h \
            $$ZINT_PATH/backend/gb2312.h \
            $$ZINT_PATH/backend/gridmtx.h \
            $$ZINT_PATH/backend/gs1.h \
            $$ZINT_PATH/backend/large.h \
            $$ZINT_PATH/backend/maxicode.h \
            $$ZINT_PATH/backend/maxipng.h \
            $$ZINT_PATH/backend/ms_stdint.h \
            $$ZINT_PATH/backend/pdf417.h \
            $$ZINT_PATH/backend/qr.h \
            $$ZINT_PATH/backend/reedsol.h \
            $$ZINT_PATH/backend/rss.h \
            $$ZINT_PATH/backend/sjis.h \
            $$ZINT_PATH/backend/zint.h \
            $$ZINT_PATH/backend_qt4/qzint.h

SOURCES += $$ZINT_PATH/backend/2of5.c \
           $$ZINT_PATH/backend/auspost.c \
           $$ZINT_PATH/backend/aztec.c \
           $$ZINT_PATH/backend/code.c \
           $$ZINT_PATH/backend/code1.c \
           $$ZINT_PATH/backend/code128.c \
           $$ZINT_PATH/backend/code16k.c \
           $$ZINT_PATH/backend/code49.c \
           $$ZINT_PATH/backend/common.c \
           $$ZINT_PATH/backend/composite.c \
           $$ZINT_PATH/backend/dmatrix.c \
           $$ZINT_PATH/backend/gridmtx.c \
           $$ZINT_PATH/backend/gs1.c \
           $$ZINT_PATH/backend/imail.c \
           $$ZINT_PATH/backend/large.c \
           $$ZINT_PATH/backend/library.c \
           $$ZINT_PATH/backend/maxicode.c \
           $$ZINT_PATH/backend/medical.c \
           $$ZINT_PATH/backend/pdf417.c \
           $$ZINT_PATH/backend/plessey.c \
           $$ZINT_PATH/backend/postal.c \
           $$ZINT_PATH/backend/ps.c \
           $$ZINT_PATH/backend/qr.c \
           $$ZINT_PATH/backend/reedsol.c \
           $$ZINT_PATH/backend/render.c \
           $$ZINT_PATH/backend/rss.c \
           $$ZINT_PATH/backend/svg.c \
           $$ZINT_PATH/backend/telepen.c \
           $$ZINT_PATH/backend/upcean.c \
           $$ZINT_PATH/backend/dllversion.c \
           $$ZINT_PATH/backend/png.c \
           $$ZINT_PATH/backend_qt4/qzint.cpp
