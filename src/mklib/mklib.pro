MKROOT = ../..
include( $${MKROOT}/mk.pri )
TEMPLATE = lib
TARGET = mklib$${SUFFIX_STR}
DESTDIR = $${MKROOT}/lib
QT += xml
CONFIG -= rtti \
    exceptions
CONFIG += static
include( $${MKROOT}/src/qwt.pri )
include( $${MKROOT}/src/modbuslib.pri )
PRECOMPILED_HEADER = pch.h
HEADERS = mbconfigwidget.h \
    mbmasterwidget.h \
    consolewidget.h \
    mbmasterxml.h \
    mktable.h \
    mktable_p.h \
    plot.h \
    helpwidget.h \
    slotwidget.h \
    slotconfigtableview.h \
    scaledraw.h \
    mkpicker.h
SOURCES = mbconfigwidget.cpp \
    mbmasterwidget.cpp \
    consolewidget.cpp \
    mbmasterxml.cpp \
    mktable.cpp \
    plot.cpp \
    helpwidget.cpp \
    slotwidget.cpp \
    slotconfigtableview.cpp \
    mkpicker.cpp
FORMS = ui/mbconfigwidget.ui \
    ui/mbmasterwidget.ui \
    ui/mbmslotexportdialog.ui \
    ui/plot.ui \
    ui/consolewidget.ui \
    ui/helpwidget.ui \
    ui/slotwidget.ui \
    ui/slotdialog.ui
RESOURCES = mklib.qrc
