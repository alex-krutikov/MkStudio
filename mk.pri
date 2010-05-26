CONFIG(debug, debug|release) { 
    SUFFIX_STR = _d \
        }
    else { 
        SUFFIX_STR = }
        UI_HEADERS_DIR = build/uic
        MOC_DIR = build/moc
        OBJECTS_DIR = build/obj$${SUFFIX_STR}
        RCC_DIR = build/rcc
        INCLUDEPATH *= .
        DEPENDEPATH *= .
