MKROOT = ../..

include( $${MKROOT}/mk.pri )

TEMPLATE = app

TARGET  = mkstudio$${SUFFIX_STR}
DESTDIR = $${MKROOT}/bin

include( $${MKROOT}/src/modbuslib.pri )
include( $${MKROOT}/src/mklib.pri )
include( $${MKROOT}/src/qwt.pri )

QT      += xml widgets network

PRECOMPILED_HEADER = pch.h

RESOURCES = mkstudio.qrc

win32{  RC_FILE = icon.rc }

# Input
HEADERS = main.h   mainwindow.h   dialogs.h   misc.h
SOURCES = main.cpp mainwindow.cpp dialogs.cpp misc.cpp
FORMS   = ui/mainwindow.ui     \
          ui/assigndialog.ui   \
          ui/settingsdialog.ui \
          ui/initdialog.ui     \
          ui/texteditdialog.ui \
          ui/unitedslots.ui    \
          ui/shortcuts.ui
