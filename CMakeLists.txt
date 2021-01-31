cmake_minimum_required(VERSION 3.1)

set(QWT_PLOT ON)
set(QWT_DISABLE_WARNINGS ON)

project(mk)

add_subdirectory(src/3rdparty/qwt/src)
add_subdirectory(src/modbuslib)
add_subdirectory(src/mklib)
add_subdirectory(src/mkstudio)
add_subdirectory(src/mmpm)
add_subdirectory(src/mkview)
add_subdirectory(src/baseedit)
add_subdirectory(src/mkserver)
add_subdirectory(src/mkserverd)
add_subdirectory(src/mksync)
add_subdirectory(src/mkquery)

