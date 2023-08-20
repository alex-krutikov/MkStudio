cmake_minimum_required(VERSION 3.15)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(mkview WIN32)

target_compile_features(mkview PRIVATE cxx_std_17)

target_sources(mkview
    PRIVATE
        icon.rc
        interface.h
        main.cpp
        main.h
        mainwindow.cpp
        mainwindow.h
        mkview.qrc
)

target_link_libraries(mkview
    modbuslib
    mklib
    qwt6
    Qt5::Widgets
)

install(TARGETS mkview DESTINATION .)
