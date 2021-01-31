cmake_minimum_required(VERSION 3.1)

set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Core Network REQUIRED)


add_library(modbuslib)

target_compile_features(modbuslib PRIVATE cxx_std_17)

target_include_directories(modbuslib INTERFACE . )

target_sources(modbuslib
    PRIVATE
        abstractmbserver.h
        abstractserialport.h
        console.cpp
        console.h
        crc.cpp
        crc.h
        mbcommon.cpp
        mbcommon.h
        mbl_global.h
        mbmaster.cpp
        mbmaster.h
        mbmaster_p.h
        mbtcp.cpp
        mbtcp.h
        mbtcpserver.cpp
        mbtcpserver.h
        mbtypes.h
        mbudp.cpp
        mbudp.h
        mbudpserver.cpp
        mbudpserver.h
        packed_struct_begin.h
        packed_struct_end.h
        serialport.cpp
        serialport.h
        serialport_p.h
        utils.cpp
        utils.h
        xfiles.cpp
        xfiles.h
)

if (WIN32)
     target_sources(modbuslib PRIVATE  serialport_win.cpp)
endif()

if (UNIX)
     target_sources(modbuslib PRIVATE  serialport_unix.cpp)
endif()

target_link_libraries(modbuslib
    Qt5::Core
    Qt5::Network
)
