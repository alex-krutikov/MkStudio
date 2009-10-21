TEMPLATE = lib
DESTDIR  = ../../lib
DLLDESTDIR  = ../../bin

#win32{ QMAKE_POST_LINK  +=  cd .. }
#win32{ QMAKE_POST_LINK  +=  && cd mkstudio && mingw32-make release && cd .. }

QT += xml
CONFIG -= rtti exceptions
unix { CONFIG += static }

DEFINES += MKLIB_DLL
CONFIG += static

include( ../qwt.pri )
include( ../modbuslib.pri )

INCLUDEPATH  += .
DEPENDPATH   += .

OBJECTS_DIR    = build
UI_HEADERS_DIR = build
MOC_DIR        = build
RCC_DIR        = build
UI_HEADERS_DIR = build

PRECOMPILED_HEADER = pch.h

HEADERS = mbconfigwidget.h                      \
          mbmasterwidget.h                      \
          consolewidget.h                       \
          mbmasterxml.h                         \
          mktable.h                             \
          mktable_p.h                           \
          plot.h                                \
          helpwidget.h                          \
          slotwidget.h                          \
          slotconfigtableview.h                 \
          
          
SOURCES = mbconfigwidget.cpp                    \
          mbmasterwidget.cpp                    \
          consolewidget.cpp                     \
          mbmasterxml.cpp                       \
          mktable.cpp                           \
          plot.cpp                              \
          helpwidget.cpp                        \
          slotwidget.cpp                        \
          slotconfigtableview.cpp               \

FORMS   = ui/mbconfigwidget.ui                  \
          ui/mbmasterwidget.ui                  \
          ui/mbmslotexportdialog.ui             \
          ui/plot.ui                            \
          ui/consolewidget.ui                   \
          ui/helpwidget.ui                      \
          ui/slotwidget.ui                      \
          ui/slotdialog.ui

RESOURCES = mklib.qrc
