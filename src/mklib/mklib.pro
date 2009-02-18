QT += xml

OBJECTS_DIR    = build
UI_HEADERS_DIR = build
MOC_DIR        = build
RCC_DIR        = build

CONFIG -= rtti exceptions

TEMPLATE = lib
DESTDIR  = ../../lib
DLLDESTDIR  = ../../bin

unix { CONFIG += static }

include( ../qwt.pri )

PRECOMPILED_HEADER = pch.h
UI_HEADERS_DIR = ui

INCLUDEPATH += .
DEPENDPATH  += . ui

HEADERS += mbconfigwidget.h
SOURCES += mbconfigwidget.cpp
FORMS   += ui/mbconfigwidget.ui

HEADERS += mbmasterwidget.h
SOURCES += mbmasterwidget.cpp
FORMS   += ui/mbmasterwidget.ui
FORMS   += ui/mbmslotwidget.ui
FORMS   += ui/mbmslotexportdialog.ui
FORMS   += ui/plot.ui

HEADERS += consolewidget.h
SOURCES += consolewidget.cpp
FORMS   += ui/consolewidget.ui

HEADERS += mbtypes.h
HEADERS += mbmaster.h
SOURCES += mbmaster.cpp

HEADERS += mbcommon.h
SOURCES += mbcommon.cpp

HEADERS += crc.h
SOURCES += crc.cpp

HEADERS += serialport.h serialport_p.h
SOURCES += serialport.cpp
win32{ SOURCES += serialport_win.cpp }
unix{  SOURCES += serialport_unix.cpp }

HEADERS += mktable.h
SOURCES += mktable.cpp

HEADERS += mkselfrecorder.h
SOURCES += mkselfrecorder.cpp

HEADERS += helpwidget.h
SOURCES += helpwidget.cpp
FORMS   += ui/helpwidget.ui

HEADERS += slotwidget.h
SOURCES += slotwidget.cpp
FORMS   += slotwidget.ui slotdialog.ui

RESOURCES += mklib.qrc

