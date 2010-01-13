MKROOT = ../..

include( $${MKROOT}/mk.pri )

TEMPLATE = app

TARGET  = mkview$${SUFFIX_STR}
DESTDIR = $${MKROOT}/bin

include( $${MKROOT}/src/modbuslib.pri )
include( $${MKROOT}/src/mklib.pri )
include( $${MKROOT}/src/qwt.pri )

QT      += xml network

PRECOMPILED_HEADER = pch.h

RESOURCES += mkview.qrc

win32{  RC_FILE = icon.rc }

# Input
HEADERS += main.h mainwindow.h interface.h
FORMS   += ui/initdialog.ui ui/midialog.ui ui/mainwindowxml.ui
SOURCES += main.cpp mainwindow.cpp

