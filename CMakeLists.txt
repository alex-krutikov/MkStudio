cmake_minimum_required(VERSION 3.7)

set(QWT_PLOT ON)

project(mk)

add_subdirectory(src/3rdparty/qwt/src)
add_subdirectory(src/modbuslib)
add_subdirectory(src/mklib)
add_subdirectory(src/mkstudio)

