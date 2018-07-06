#-------------------------------------------------
#
# Project created by QtCreator 2018-07-04T11:08:09
#
#-------------------------------------------------

QT       += widgets network gui

TARGET = qssh
TEMPLATE = lib
DEFINES += QSSH_LIBRARY
CONFIG += staticlib

PRE_TARGETDEPS = ../botan2/libbotan2.a
INCLUDEPATH += ../botan2

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES = sshsendfacility.cpp \
    sshremoteprocess.cpp \
    sshpacketparser.cpp \
    sshpacket.cpp \
    sshoutgoingpacket.cpp \
    sshkeygenerator.cpp \
    sshkeyexchange.cpp \
    sshincomingpacket.cpp \
    sshcryptofacility.cpp \
    sshconnection.cpp \
    sshchannelmanager.cpp \
    sshchannel.cpp \
    sshcapabilities.cpp \
    sftppacket.cpp \
    sftpoutgoingpacket.cpp \
    sftpoperation.cpp \
    sftpincomingpacket.cpp \
    sftpdefs.cpp \
    sftpchannel.cpp \
    sshremoteprocessrunner.cpp \
    sshconnectionmanager.cpp \
    sshkeypasswordretriever.cpp \
    sftpfilesystemmodel.cpp

HEADERS = sshsendfacility_p.h \
    sshremoteprocess.h \
    sshremoteprocess_p.h \
    sshpacketparser_p.h \
    sshpacket_p.h \
    sshoutgoingpacket_p.h \
    sshkeygenerator.h \
    sshkeyexchange_p.h \
    sshincomingpacket_p.h \
    sshexception_p.h \
    ssherrors.h \
    sshcryptofacility_p.h \
    sshconnection.h \
    sshconnection_p.h \
    sshchannelmanager_p.h \
    sshchannel_p.h \
    sshcapabilities_p.h \
    sshbotanconversions_p.h \
    sftppacket_p.h \
    sftpoutgoingpacket_p.h \
    sftpoperation_p.h \
    sftpincomingpacket_p.h \
    sftpdefs.h \
    sftpchannel.h \
    sftpchannel_p.h \
    sshremoteprocessrunner.h \
    sshconnectionmanager.h \
    sshpseudoterminal.h \
    sshkeypasswordretriever_p.h \
    sftpfilesystemmodel.h \
    ssh_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}



