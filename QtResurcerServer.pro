QT += core network gui widgets

TARGET = QtResurcerServer
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += debug_and_release

HEADERS += \
    IResourcesServer.h \
    config.h \
    resource_controller.h \
    server_worker.h \
    server_window.h

SOURCES += \
    IResourcesServer.cpp \
    config.cpp \
    resource_controller.cpp \
    server_main.cpp \
    server_worker.cpp \
    server_window.cpp

FORMS += \
    server_window.ui

RESOURCES += \
    resource.qrc
