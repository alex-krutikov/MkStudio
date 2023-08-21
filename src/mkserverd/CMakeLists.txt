cmake_minimum_required(VERSION 3.15)

find_package(Qt6 COMPONENTS Core Core5Compat REQUIRED)

add_executable(mkserverd)

target_compile_features(mkserverd PRIVATE cxx_std_17)

target_sources(mkserverd
    PRIVATE
        mkserverd.cpp
)

target_link_libraries(mkserverd
    modbuslib
    Qt6::Core
    Qt6::Core5Compat
)

install(TARGETS mkserverd DESTINATION .)
