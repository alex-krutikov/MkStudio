cmake_minimum_required(VERSION 3.15)

find_package(Qt5 COMPONENTS Core REQUIRED)

add_executable(mkserverd)

target_compile_features(mkserverd PRIVATE cxx_std_17)

target_sources(mkserverd
    PRIVATE
        mkserverd.cpp
)

target_link_libraries(mkserverd
    modbuslib
    Qt5::Core
)

install(TARGETS mkserverd DESTINATION .)
