include( ../mklib.pri )

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../mkview ui
DEPENDPATH     += ../mkview ui
DESTDIR         = ../../bin
HEADERS         = plugin.h
SOURCES         = plugin_mai100.cpp
FORMS           = ui/mainwindow.ui
TARGET          = mkview_mai100
CONFIG         -= rtti exceptions

