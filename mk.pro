TEMPLATE = subdirs
SUBDIRS  = src

# NSIS installer

win32{
  !exists( set ) {
     system(mkdir set)
  }

  exists( $$[QT_INSTALL_BINS]/Qt*.dll ) {
    NSIS_QT_DLL = /DQT_DLL_DIR=$$[QT_INSTALL_BINS]
  }

  nsis_setup.depends   = release
  nsis_setup.commands  = makensis.exe /DMKROOT=$${PWD} /DOUTFILE=mkstudio-setup.exe $${NSIS_QT_DLL} $${PWD}/src/nsis/mkstudio.nsi

  !isEmpty(NSIS_QT_DLL) {
    nsis_upgrade.depends  = release
    nsis_upgrade.commands = makensis.exe /DMKROOT=$${PWD} /DOUTFILE=mkstudio-upgrade.exe $${PWD}/src/nsis/mkstudio.nsi
  }

  nsis.depends  = nsis_setup nsis_upgrade

  QMAKE_EXTRA_TARGETS = nsis_setup nsis_upgrade nsis
}
