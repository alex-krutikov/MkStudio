win32{
INCLUDEPATH += $$[QT_INSTALL_PREFIX]/include/Qwt
LIBS        += -lqwt5 
}

unix{
INCLUDEPATH += /usr/local/qwt/include
LIBS        += -lqwt
}
