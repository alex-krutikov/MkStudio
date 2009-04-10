include( ../modbuslib.pri )

QT += network

TEMPLATE = app
TARGET   = mkserver
DEPENDPATH  += . build
INCLUDEPATH += . build

OBJECTS_DIR    = build
UI_HEADERS_DIR = build
MOC_DIR        = build
DESTDIR        = build
RCC_DIR        = build

DESTDIR = ../../bin

CONFIG -= exceptions rtti

TRANSLATIONS = qt_ru.ts2

PRECOMPILED_HEADER = pch.h
RESOURCES          = mkserver.qrc
win32 { RC_FILE    = icon.rc }

FORMS   = ui/mainwindow.ui

HEADERS = mainwindow.h           \
          main.h                 \
          server.h               \

SOURCES = mainwindow.cpp         \
          main.cpp               \
          server.cpp             \

