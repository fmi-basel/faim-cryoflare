TEMPLATE = subdirs

SUBDIRS = limereport designer
designer.depends = limereport
!contains(CONFIG, no_zint){
    CONFIG += zint
}

include(common.pri)
contains(CONFIG, zint){
    SUBDIRS += 3rdparty
    limereport.depends = 3rdparty
}

export($$CONFIG)




