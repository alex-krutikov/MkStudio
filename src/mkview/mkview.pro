include( ../mklib.pri )

TEMPLATE = app
QT      += xml
DEPENDPATH  += . ui
INCLUDEPATH += . ui

#####QMAKE_CXXFLAGS_RELEASE -= -O2 -s
#####QMAKE_CXXFLAGS_RELEASE += -O0 -g
#####QMAKE_LFLAGS_RELEASE   -= -Wl,-s

release:DESTDIR    = ../../bin

PRECOMPILED_HEADER = pch.h
UI_HEADERS_DIR = ui

RESOURCES += mkview.qrc

win32{  RC_FILE = icon.rc }

# Input
HEADERS += main.h mainwindow.h interface.h
FORMS   += ui/initdialog.ui ui/midialog.ui ui/mainwindowxml.ui
SOURCES += main.cpp mainwindow.cpp

