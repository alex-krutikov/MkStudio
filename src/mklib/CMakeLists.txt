cmake_minimum_required(VERSION 3.15)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)


find_package(Qt6 COMPONENTS Widgets Xml Core5Compat REQUIRED)


add_library(mklib)

target_compile_features(mklib PRIVATE cxx_std_17)

target_include_directories(mklib PRIVATE . INTERFACE . )

target_sources(mklib
    PRIVATE
        consolewidget.cpp
        consolewidget.h
        helpwidget.cpp
        helpwidget.h
        mbconfigwidget.cpp
        mbconfigwidget.h
        mbmasterwidget.cpp
        mbmasterwidget.h
        mbmasterxml.cpp
        mbmasterxml.h
        mk_global.h
        mklib.qrc
        mkpicker.cpp
        mkpicker.h
        mktable.cpp
        mktable.h
        mktable_p.h
        plot.cpp
        plot.h
        scaledraw.h
        shortcut.h
        slotconfigtableview.cpp
        slotconfigtableview.h
        slotwidget.cpp
        slotwidget.h
)

target_link_libraries(mklib
    modbuslib
    qwt6
    Qt6::Widgets
    Qt6::Xml
    Qt6::Core5Compat
)
