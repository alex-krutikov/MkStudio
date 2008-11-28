include( ../mklib.pri )

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += . ../mkview ui
DEPENDPATH     += . ui
DESTDIR         = ../../bin
HEADERS         = plugin.h  speedo_meter.h
SOURCES         = plugin_tb.cpp speedo_meter.cpp
FORMS           = ui/mainwindow.ui
TARGET          = mkview_ai400
CONFIG         -= rtti exceptions
UI_HEADERS_DIR  = ui
RESOURCES       = plugin.qrc

