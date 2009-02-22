INCLUDEPATH += ../modbuslib
DEPENDPATH  += ../modbuslib
LIBS += -L../../lib -lmodbuslib

PRE_TARGETDEPS = ../../lib/libmodbuslib.a

CONFIG -= rtti exceptions
