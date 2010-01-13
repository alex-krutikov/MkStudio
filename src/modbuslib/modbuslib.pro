MKROOT = ../..

include( $${MKROOT}/mk.pri )

TEMPLATE = lib

TARGET = modbuslib$${SUFFIX_STR}
DESTDIR  = $${MKROOT}/lib

QT -= gui
QT += network
CONFIG -= rtti exceptions
CONFIG += static

PRECOMPILED_HEADER = pch.h

HEADERS = mbmaster.h                            \
          mbmaster_p.h                          \
          serialport.h                          \
          mbtcpserver.h                         \

SOURCES = mbmaster.cpp                          \
          mbcommon.cpp                          \
          crc.cpp                               \
          serialport.cpp                        \
          console.cpp                           \
          xfiles.cpp                            \
          mbtcp.cpp                             \
          mbtcpserver.cpp                       \

win32{ SOURCES += serialport_win.cpp  }
unix{  SOURCES += serialport_unix.cpp }
