MKROOT = ../..
include( $${MKROOT}/mk.pri )

TEMPLATE = app

TARGET  = mkserver$${SUFFIX_STR}
DESTDIR = $${MKROOT}/bin

include( $${MKROOT}/src/modbuslib.pri )

QT += network

CONFIG -= exceptions rtti

TRANSLATIONS = qt_ru.ts2

PRECOMPILED_HEADER = pch.h
RESOURCES          = mkserver.qrc
win32 { RC_FILE    = icon.rc }

FORMS   = ui/mainwindow.ui

HEADERS = mainwindow.h           \
          main.h                 \

SOURCES = mainwindow.cpp         \
          main.cpp               \

