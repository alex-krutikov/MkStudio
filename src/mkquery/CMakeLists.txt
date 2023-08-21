cmake_minimum_required(VERSION 3.15)

find_package(Qt6 COMPONENTS Core Core5Compat REQUIRED)

add_executable(mkquery)

target_compile_features(mkquery PRIVATE cxx_std_17)

target_sources(mkquery
    PRIVATE
        mkquery.cpp
)

target_link_libraries(mkquery
    modbuslib
    Qt6::Core
    Qt6::Core5Compat
)

install(TARGETS mkquery DESTINATION .)
