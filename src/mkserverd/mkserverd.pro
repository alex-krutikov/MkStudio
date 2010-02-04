MKROOT = ../..
include( $${MKROOT}/mk.pri )

TEMPLATE = app

TARGET  = mkserverd$${SUFFIX_STR}
DESTDIR = $${MKROOT}/bin

include( $${MKROOT}/src/modbuslib.pri )

QT -= gui
QT += network
CONFIG += console

# Input
SOURCES = mkserverd.cpp
