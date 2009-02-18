include( ../mklib.pri )

OBJECTS_DIR    = build
UI_HEADERS_DIR = build
MOC_DIR        = build
RCC_DIR        = build
UI_HEADERS_DIR = build

TEMPLATE = app
QT      += xml
DEPENDPATH  += . ui
INCLUDEPATH += . ui

### CONFIG += console

DESTDIR = ../../bin

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
          ui/unitedslots.ui
