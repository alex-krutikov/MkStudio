cmake_minimum_required(VERSION 3.7)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(mmpm WIN32)

target_compile_features(mmpm PRIVATE cxx_std_17)

target_include_directories(mmpm PRIVATE . INTERFACE . )

target_sources(mmpm
    PRIVATE
        main.cpp
        main.h
        mainwindow.cpp
        mainwindow.h
        misc.cpp
        misc.h
        mmpm.qrc
        mmpm.rc
        mmpm_types.h
        modbus.cpp
        modbus.h
        thread.cpp
        thread.h
)

target_link_libraries(mmpm
    modbuslib
    mklib
    Qt5::Widgets
)

install(TARGETS mmpm DESTINATION .)
