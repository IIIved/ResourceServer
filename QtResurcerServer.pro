QT += core network gui widgets

TARGET = QtResurcerServer
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += debug_and_release

HEADERS += \
    IResourcesServer.h \
    ResourceController.h \
    ServerWindow.h \
    ServerWorker.h \
    config.h \
    resourceserver.h

SOURCES += \
    ResourceController.cpp \
    ServerMain.cpp \
    ServerWindow.cpp \
    ServerWorker.cpp \
    config.cpp \
    resourceserver.cpp

FORMS += \
    ServerWindow.ui

RESOURCES += \
    resource.qrc
