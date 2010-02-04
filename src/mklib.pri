INCLUDEPATH += $${MKROOT}/src/mklib
DEPENDPATH  += $${MKROOT}/src/mklib
LIBS += -L$${MKROOT}/lib -lmklib$${SUFFIX_STR}

include( qwt.pri )
