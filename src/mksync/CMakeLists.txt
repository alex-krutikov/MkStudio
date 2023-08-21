cmake_minimum_required(VERSION 3.15)

find_package(Qt6 COMPONENTS Core Xml Core5Compat REQUIRED)

add_executable(mksync)

target_compile_features(mksync PRIVATE cxx_std_17)

target_sources(mksync
    PRIVATE
        mksync.cpp
)

target_link_libraries(mksync
    modbuslib
    Qt6::Core
    Qt6::Xml
    Qt6::Core5Compat
)

install(TARGETS mksync DESTINATION .)
