include( ../mklib.pri )

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../mkview ui
DEPENDPATH     += ../mkview ui
DESTDIR         = ../../bin
HEADERS         = plugin.h
SOURCES         = plugin_mai103.cpp
FORMS           = ui/mainwindow.ui
TARGET          = mkview_mai103
CONFIG         -= rtti exceptions

