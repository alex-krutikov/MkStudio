cmake_minimum_required(VERSION 3.7)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)


add_executable(mkstudio WIN32)

target_compile_features(mkstudio PRIVATE cxx_std_17)

if(CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(mklib
        PRIVATE
            -Wno-deprecated-declarations
    )
endif()

target_include_directories(mkstudio PRIVATE . INTERFACE . )

target_sources(mkstudio
    PRIVATE
        dialogs.cpp
        dialogs.h
        icon.rc
        info.h
        main.cpp
        mainwindow.cpp
        mainwindow.h
        misc.cpp
        misc.h
        mkstudio.qrc
        pch.h
        utils.h
)

target_link_libraries(mkstudio
    modbuslib
    mklib
    Qt5::Widgets
)

install(TARGETS mkstudio DESTINATION .)
