win32{
INCLUDEPATH += $$[QT_INSTALL_PREFIX]/include/Qwt

CONFIG(debug,debug|release)   : LIBS    += -lqwtd5 
CONFIG(release,debug|release) : LIBS    += -lqwt5 
}

unix{
INCLUDEPATH += /usr/local/qwt/include
LIBS        += -lqwt
}
