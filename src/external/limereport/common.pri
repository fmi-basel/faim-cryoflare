CONFIG += build_translations

!contains(CONFIG, no_zint){
    CONFIG += zint
}

ZINT_PATH = $$PWD/3rdparty/zint-2.4.4
contains(CONFIG,zint){
    DEFINES += HAVE_ZINT
}

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += uitools
}
lessThan(QT_MAJOR_VERSION, 5){
    CONFIG += uitools
}

CONFIG(release, debug|release){
    BUILD_TYPE = release
}else{
    BUILD_TYPE = debug
}


LIMEREPORT_VERSION_MAJOR = 1
LIMEREPORT_VERSION_MINOR = 4
LIMEREPORT_VERSION_RELEASE = 83

LIMEREPORT_VERSION = '\\"$${LIMEREPORT_VERSION_MAJOR}.$${LIMEREPORT_VERSION_MINOR}.$${LIMEREPORT_VERSION_RELEASE}\\"'
DEFINES += LIMEREPORT_VERSION_STR=\"$${LIMEREPORT_VERSION}\"
DEFINES += LIMEREPORT_VERSION=$${LIMEREPORT_VERSION}

QT += script xml sql

greaterThan(QT_MAJOR_VERSION, 4) {
    DEFINES+=HAVE_QT5
    QT+= printsupport widgets
    contains(QT,uitools){
        DEFINES += HAVE_UI_LOADER
    }
}

lessThan(QT_MAJOR_VERSION, 5){
    DEFINES+=HAVE_QT4
    CONFIG(uitools){
        DEFINES += HAVE_UI_LOADER
    }
}


