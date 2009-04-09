TEMPLATE = lib
DESTDIR  = ../../lib
DLLDESTDIR  = ../../bin

win32{ QMAKE_POST_LINK  +=  cd .. }
win32{ QMAKE_POST_LINK  +=  && cd mklib    && mingw32-make release && cd .. }
win32{ QMAKE_POST_LINK  +=  && cd mkstudio && mingw32-make release && cd .. }

QT -= gui
QT += network
CONFIG -= rtti exceptions
unix { CONFIG += static }

CONFIG += static

INCLUDEPATH  = .
DEPENDPATH   = .

OBJECTS_DIR    = build
UI_HEADERS_DIR = build
MOC_DIR        = build
RCC_DIR        = build
UI_HEADERS_DIR = build

PRECOMPILED_HEADER = pch.h

HEADERS = mbmaster.h                            \
          mbmaster_p.h                          \

SOURCES = mbmaster.cpp                          \
          mbcommon.cpp                          \
          crc.cpp                               \
          serialport.cpp                        \
          console.cpp                           \
          xfiles.cpp                            \
          mbtcp.cpp                             \

win32{ SOURCES += serialport_win.cpp  }
unix{  SOURCES += serialport_unix.cpp }
