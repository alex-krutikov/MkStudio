include( ../mklib.pri )

TEMPLATE = app
QT      += xml
DEPENDPATH  += . ui
INCLUDEPATH += . ui

### CONFIG += console

DESTDIR = ../../bin

PRECOMPILED_HEADER = pch.h
UI_HEADERS_DIR = ui

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
