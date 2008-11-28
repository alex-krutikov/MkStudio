include( ../mklib.pri )

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../mkview ui
DEPENDPATH     += . ui
DESTDIR         = ../../bin
HEADERS         = plugin.h
SOURCES         = plugin_di102.cpp
FORMS           = ui/mainwindow.ui
TARGET          = mkview_di102
CONFIG         -= rtti exceptions
UI_HEADERS_DIR = ui

