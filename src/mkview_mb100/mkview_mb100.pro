include( ../mklib.pri )
QT += xml

#####QMAKE_CXXFLAGS_RELEASE -= -O2
#####QMAKE_CXXFLAGS_RELEASE += -O0 -g
#####QMAKE_LFLAGS_RELEASE   -= -Wl,-s

TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../mkview ui
DEPENDPATH     += . ui
DESTDIR         = ../../bin
HEADERS         = plugin.h
SOURCES         = plugin_mb100.cpp
FORMS           = ui/mainwindow.ui
RESOURCES       = plugin.qrc
TARGET          = mkview_mb100
CONFIG         -= rtti exceptions
UI_HEADERS_DIR = ui

