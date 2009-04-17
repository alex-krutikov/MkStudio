include( ../modbuslib.pri )

OBJECTS_DIR    = build
UI_HEADERS_DIR = build
MOC_DIR        = build
RCC_DIR        = build
UI_HEADERS_DIR = build

DESTDIR = ../../bin

QT -= gui
QT += network
CONFIG += console

TEMPLATE = app
TARGET   = mkquery

# Input
SOURCES = mkquery.cpp
