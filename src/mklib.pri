INCLUDEPATH += ../mklib ../mklib/build
DEPENDPATH  += ../mklib ../mklib/build
LIBS += -L../../lib -lmklib

PRE_TARGETDEPS = ../../lib/libmklib.a

include( qwt.pri )

CONFIG -= rtti exceptions
