INCLUDEPATH += ../mklib
DEPENDPATH  += ../mklib
LIBS += -L../../lib -lmklib

PRE_TARGETDEPS += ../../lib/libmklib.a

include( qwt.pri )

CONFIG -= rtti exceptions
