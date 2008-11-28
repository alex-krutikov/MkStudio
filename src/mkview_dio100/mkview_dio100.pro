include( ../mklib.pri )

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../mkview ui
DEPENDPATH     += . ui
DESTDIR         = ../../bin
HEADERS         = plugin.h
SOURCES         = plugin_dio100.cpp
FORMS           = ui/mainwindow.ui
TARGET          = mkview_dio100
CONFIG         -= rtti exceptions
UI_HEADERS_DIR = ui

