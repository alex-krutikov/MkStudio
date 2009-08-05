win32{
INCLUDEPATH += $$[QT_INSTALL_PREFIX]/include/Qwt
debug:LIBS    += -lqwtd5 
release:LIBS  += -lqwt5 
}

unix{
INCLUDEPATH += /usr/local/qwt/include
LIBS        += -lqwt
}
