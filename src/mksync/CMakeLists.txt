cmake_minimum_required(VERSION 3.1)

find_package(Qt5 COMPONENTS Core Xml REQUIRED)

add_executable(mksync)

target_compile_features(mksync PRIVATE cxx_std_17)

target_sources(mksync
    PRIVATE
        mksync.cpp
)

target_link_libraries(mksync
    modbuslib
    Qt5::Core
    Qt5::Xml
)

install(TARGETS mksync DESTINATION .)
