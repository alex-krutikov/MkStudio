include( ../mklib.pri )

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../mkview ui
DEPENDPATH     += . ui
DESTDIR         = ../../bin
HEADERS         = plugin.h
SOURCES         = plugin_nodetest.cpp
TARGET          = mkview_nodetest
CONFIG         -= rtti exceptions
UI_HEADERS_DIR = ui

