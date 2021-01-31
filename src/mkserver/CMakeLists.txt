cmake_minimum_required(VERSION 3.7)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Widgets Xml REQUIRED)

add_executable(mkserver WIN32)

target_compile_features(mkserver PRIVATE cxx_std_17)

target_sources(mkserver
    PRIVATE
        icon.rc
        main.cpp
        main.h
        mainwindow.cpp
        mainwindow.h
        mkserver.qrc
)

target_link_libraries(mkserver
    modbuslib
    Qt5::Widgets
    Qt5::Xml
)

install(TARGETS mkserver DESTINATION .)
