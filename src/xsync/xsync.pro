include( ../modbuslib.pri )

OBJECTS_DIR    = build
UI_HEADERS_DIR = build
MOC_DIR        = build
RCC_DIR        = build
UI_HEADERS_DIR = build

DESTDIR = ../../bin

QT -= gui
CONFIG += console

TEMPLATE = app
TARGET   = xsync

# Input
SOURCES = xsync.cpp
