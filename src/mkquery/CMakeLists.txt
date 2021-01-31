cmake_minimum_required(VERSION 3.7)

find_package(Qt5 COMPONENTS Core REQUIRED)

add_executable(mkquery)

target_compile_features(mkquery PRIVATE cxx_std_17)

target_sources(mkquery
    PRIVATE
        mkquery.cpp
)

target_link_libraries(mkquery
    modbuslib
    Qt5::Core
)

install(TARGETS mkquery DESTINATION .)
