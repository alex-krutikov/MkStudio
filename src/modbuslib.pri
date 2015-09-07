INCLUDEPATH += $${MKROOT}/src/modbuslib
DEPENDPATH  += $${MKROOT}/src/modbuslib
LIBS += -L$${MKROOT}/lib -lmodbuslib$${SUFFIX_STR}

MKROOT_SHADOWED = $$shadowed($${PWD}$${MKROOT})
PRE_TARGETDEPS += $${MKROOT_SHADOWED}/lib/$${QMAKE_PREFIX_STATICLIB}modbuslib$${SUFFIX_STR}.$${QMAKE_EXTENSION_STATICLIB}

win32 {
  LIBS += -luser32 -ladvapi32
}
