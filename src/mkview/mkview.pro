include( ../mklib.pri )

OBJECTS_DIR    = build
UI_HEADERS_DIR = build
MOC_DIR        = build
RCC_DIR        = build
UI_HEADERS_DIR = build

TEMPLATE = app
QT      += xml
DEPENDPATH  += . ui
INCLUDEPATH += . ui

#####QMAKE_CXXFLAGS_RELEASE -= -O2 -s
#####QMAKE_CXXFLAGS_RELEASE += -O0 -g
#####QMAKE_LFLAGS_RELEASE   -= -Wl,-s

release:DESTDIR    = ../../bin

PRECOMPILED_HEADER = pch.h

RESOURCES += mkview.qrc

win32{  RC_FILE = icon.rc }

# Input
HEADERS += main.h mainwindow.h interface.h
FORMS   += ui/initdialog.ui ui/midialog.ui ui/mainwindowxml.ui
SOURCES += main.cpp mainwindow.cpp

