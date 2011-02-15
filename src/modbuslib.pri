INCLUDEPATH += $${MKROOT}/src/modbuslib
DEPENDPATH  += $${MKROOT}/src/modbuslib
LIBS += -L$${MKROOT}/lib -lmodbuslib$${SUFFIX_STR}

win32 {
  LIBS += -luser32 -ladvapi32
}