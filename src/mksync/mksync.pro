MKROOT = ../..
include( $${MKROOT}/mk.pri )

TEMPLATE = app

TARGET  = mksync$${SUFFIX_STR}
DESTDIR = $${MKROOT}/bin

include( $${MKROOT}/src/modbuslib.pri )

QT -= gui
QT += network
QT += xml
CONFIG += console

# Input
SOURCES = mksync.cpp
