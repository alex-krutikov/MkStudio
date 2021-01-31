cmake_minimum_required(VERSION 3.7)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Widgets Xml REQUIRED)

add_executable(baseedit WIN32)

target_compile_features(baseedit PRIVATE cxx_std_17)

target_sources(baseedit
    PRIVATE
        baseedit.qrc
        icon.rc
        info.h
        main.cpp
        main.h
        mainwindow.cpp
        mainwindow.h
        model.cpp
        model.h
)

target_link_libraries(baseedit
    Qt5::Widgets
    Qt5::Xml
)

install(TARGETS baseedit DESTINATION .)
