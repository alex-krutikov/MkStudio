include( ../mklib.pri )

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../mkview ui
DEPENDPATH     += . ui
DESTDIR         = ../../bin
HEADERS         = plugin.h
SOURCES         = plugin_ai100.cpp
FORMS           = ui/mainwindow.ui ui/modulewidget.ui
TARGET          = mkview_ai100
CONFIG         -= rtti exceptions
UI_HEADERS_DIR = ui

