TEMPLATE = subdirs
SUBDIRS  = src

win32{
  !exists( set ) {
     system(mkdir set)
  }

  exists( $$[QT_INSTALL_BINS]/Qt*.dll ) {
    NSIS_QT_DLL = /DQT_DLL_DIR=\"$$[QT_INSTALL_BINS]\"
  }

  nsis.depends  = release
  nsis.commands = makensis.exe /DMKROOT=\"$${PWD}\" $${NSIS_QT_DLL} $${PWD}/src/nsis/mkstudio.nsi

  QMAKE_EXTRA_TARGETS += nsis
}
