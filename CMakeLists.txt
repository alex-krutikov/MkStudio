cmake_minimum_required(VERSION 3.15)

set(QWT_PLOT ON)
set(QWT_DISABLE_WARNINGS ON)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_compile_options("$<$<CXX_COMPILER_ID:GNU>:-ffunction-sections>")
add_compile_options("$<$<CXX_COMPILER_ID:GNU>:-fdata-sections>")
add_link_options("$<$<C_COMPILER_ID:GNU>:-Wl,--gc-sections>")

# add_compile_options("$<$<CXX_COMPILER_ID:GNU>:-Werror>")

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

