cmake_minimum_required(VERSION 3.0.2)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
if (${CMAKE_VERSION} VERSION_LESS "3.17.0")
  list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake-compat)
endif()

project(libzip
  VERSION 1.8.0
  LANGUAGES C)

option(ENABLE_COMMONCRYPTO "Enable use of CommonCrypto" OFF)
option(ENABLE_GNUTLS "Enable use of GnuTLS" OFF)
option(ENABLE_MBEDTLS "Enable use of mbed TLS" OFF)
option(ENABLE_OPENSSL "Enable use of OpenSSL" OFF)
option(ENABLE_WINDOWS_CRYPTO "Enable use of Windows cryptography libraries" OFF)

option(ENABLE_BZIP2 "Enable use of BZip2" OFF)
option(ENABLE_LZMA "Enable use of LZMA" ON)
option(ENABLE_ZSTD "Enable use of Zstandard" OFF)

option(BUILD_TOOLS "Build tools in the src directory (zipcmp, zipmerge, ziptool)" OFF)
option(BUILD_REGRESS "Build regression tests" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)
option(BUILD_DOC "Build documentation" OFF)

include(CheckFunctionExists)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckSymbolExists)
include(CheckTypeSize)
include(CheckCSourceRuns)
include(CheckCSourceCompiles)
include(CheckStructHasMember)
include(TestBigEndian)
include(GNUInstallDirs)

if(ENABLE_COMMONCRYPTO)
  check_include_files(CommonCrypto/CommonCrypto.h COMMONCRYPTO_FOUND)
endif()
if(ENABLE_GNUTLS)
  find_package(Nettle 3.0)
  find_package(GnuTLS)
endif()
if(ENABLE_MBEDTLS)
  find_package(MbedTLS 1.0)
endif()
if(ENABLE_OPENSSL)
  find_package(OpenSSL)
endif()
if(WIN32)
  if(ENABLE_WINDOWS_CRYPTO)
    set(WINDOWS_CRYPTO_FOUND TRUE)
  endif()
endif()

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(LIBZIP_DO_INSTALL "Install libzip and the related files" OFF)

option(SHARED_LIB_VERSIONNING "Add SO version in .so build" ON)

find_program(MDOCTOOL NAMES mandoc groff)
if (MDOCTOOL)
  set(DOCUMENTATION_FORMAT "mdoc" CACHE STRING "Documentation format")
else()
  find_program(MANTOOL NAMES nroff)
  if (MANTOOL)
    set(DOCUMENTATION_FORMAT "man" CACHE STRING "Documentation format")
  else()
    set(DOCUMENTATION_FORMAT "html" CACHE STRING "Documentation format")
  endif()
endif()

include(Dist)
Dist(${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION})

#ADD_CUSTOM_TARGET(uninstall
#  COMMAND cat ${PROJECT_BINARY_DIR}/install_manifest.txt | xargs rm
#  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#  )

if(BUILD_SHARED_LIBS)
  set(HAVE_SHARED TRUE)
else()
  set(ZIP_STATIC TRUE)
endif()

# Checks

check_function_exists(_close HAVE__CLOSE)
check_function_exists(_dup HAVE__DUP)
check_function_exists(_fdopen HAVE__FDOPEN)
check_function_exists(_fileno HAVE__FILENO)
check_function_exists(_setmode HAVE__SETMODE)
check_symbol_exists(_snprintf stdio.h HAVE__SNPRINTF)
check_function_exists(_strdup HAVE__STRDUP)
check_symbol_exists(_stricmp string.h HAVE__STRICMP)
check_function_exists(_strtoi64 HAVE__STRTOI64)
check_function_exists(_strtoui64 HAVE__STRTOUI64)
check_function_exists(_unlink HAVE__UNLINK)
check_function_exists(arc4random HAVE_ARC4RANDOM)
check_function_exists(clonefile HAVE_CLONEFILE)
check_function_exists(explicit_bzero HAVE_EXPLICIT_BZERO)
check_function_exists(explicit_memset HAVE_EXPLICIT_MEMSET)
check_function_exists(fileno HAVE_FILENO)
check_function_exists(fseeko HAVE_FSEEKO)
check_function_exists(ftello HAVE_FTELLO)
check_function_exists(getprogname HAVE_GETPROGNAME)
check_function_exists(localtime_r HAVE_LOCALTIME_R)
check_function_exists(setmode HAVE_SETMODE)
check_symbol_exists(snprintf stdio.h HAVE_SNPRINTF)
check_symbol_exists(strcasecmp strings.h HAVE_STRCASECMP)
check_function_exists(strdup HAVE_STRDUP)
check_function_exists(stricmp HAVE_STRICMP)
check_function_exists(strtoll HAVE_STRTOLL)
check_function_exists(strtoull HAVE_STRTOULL)

check_include_files("sys/types.h;sys/stat.h;fts.h" HAVE_FTS_H)
# fts functions may be in external library
if(HAVE_FTS_H)
  check_function_exists(fts_open HAVE_FTS_OPEN)
  if(NOT HAVE_FTS_OPEN)
    check_library_exists(fts fts_open "" HAVE_LIB_FTS)
  else(NOT HAVE_FTS_OPEN)
    set(HAVE_LIB_FTS "" CACHE INTERNAL "")
  endif(NOT HAVE_FTS_OPEN)
else(HAVE_FTS_H)
  set(HAVE_LIB_FTS "" CACHE INTERNAL "")
endif(HAVE_FTS_H)

if(HAVE_LIB_FTS)
  set(FTS_LIB fts CACHE INTERNAL "")
else()
  set(FTS_LIB "" CACHE INTERNAL "")
endif()

check_include_files(stdbool.h HAVE_STDBOOL_H)
check_include_files(strings.h HAVE_STRINGS_H)
check_include_files(unistd.h HAVE_UNISTD_H)

check_include_files(inttypes.h HAVE_INTTYPES_H_LIBZIP)
check_include_files(stdint.h HAVE_STDINT_H_LIBZIP)
check_include_files(sys/types.h HAVE_SYS_TYPES_H_LIBZIP)

# TODO: fix test
# this test does not find __progname even when it exists
#check_symbol_exists(__progname stdlib.h HAVE___PROGNAME)

check_type_size(__int8 __INT8_LIBZIP)
check_type_size(int8_t INT8_T_LIBZIP)
check_type_size(uint8_t UINT8_T_LIBZIP)
check_type_size(__int16 __INT16_LIBZIP)
check_type_size(int16_t INT16_T_LIBZIP)
check_type_size(uint16_t UINT16_T_LIBZIP)
check_type_size(__int32 __INT32_LIBZIP)
check_type_size(int32_t INT32_T_LIBZIP)
check_type_size(uint32_t UINT32_T_LIBZIP)
check_type_size(__int64 __INT64_LIBZIP)
check_type_size(int64_t INT64_T_LIBZIP)
check_type_size(uint64_t UINT64_T_LIBZIP)
check_type_size("short" SHORT_LIBZIP)
check_type_size("int" INT_LIBZIP)
check_type_size("long" LONG_LIBZIP)
check_type_size("long long" LONG_LONG_LIBZIP)
check_type_size("off_t" SIZEOF_OFF_T)
check_type_size("size_t" SIZEOF_SIZE_T)

check_c_source_compiles("#include <sys/ioctl.h>
#include <linux/fs.h>
int main(int argc, char *argv[]) { unsigned long x = FICLONERANGE; }" HAVE_FICLONERANGE)

check_c_source_compiles("
int foo(char * _Nullable bar);
int main(int argc, char *argv[]) { }" HAVE_NULLABLE)

test_big_endian(WORDS_BIGENDIAN)

if(ENABLE_BZIP2)
  find_package(BZip2)
  if(BZIP2_FOUND)
    set(HAVE_LIBBZ2 1)
  else()
    message(WARNING "-- bzip2 library not found; bzip2 support disabled")
  endif(BZIP2_FOUND)
endif(ENABLE_BZIP2)

set (HAVE_LIBLZMA 1)

if(ENABLE_ZSTD)
  find_package(Zstd)
  if(Zstd_FOUND)
    set(HAVE_LIBZSTD 1)
  else()
    message(WARNING "-- zstd library not found; zstandard support disabled")
  endif(Zstd_FOUND)
endif(ENABLE_ZSTD)

if (COMMONCRYPTO_FOUND)
  set(HAVE_CRYPTO 1)
  set(HAVE_COMMONCRYPTO 1)
elseif (WINDOWS_CRYPTO_FOUND)
  set(HAVE_CRYPTO 1)
  set(HAVE_WINDOWS_CRYPTO 1)
elseif (GNUTLS_FOUND AND NETTLE_FOUND)
  set(HAVE_CRYPTO 1)
  set(HAVE_GNUTLS 1)
elseif (OPENSSL_FOUND)
  set(HAVE_CRYPTO 1)
  set(HAVE_OPENSSL 1)
elseif (MBEDTLS_FOUND)
  set(HAVE_CRYPTO 1)
  set(HAVE_MBEDTLS 1)
endif()

if (NOT HAVE_CRYPTO)
  message("-- neither Common Crypto, GnuTLS, mbed TLS, OpenSSL, nor Windows Cryptography found; AES support disabled")
endif()

if(MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
  add_compile_definitions(_CRT_NONSTDC_NO_DEPRECATE)
endif(MSVC)

if(WIN32)
  if(CMAKE_SYSTEM_NAME MATCHES WindowsPhone OR CMAKE_SYSTEM_NAME MATCHES WindowsStore)
    add_compile_definitions(MS_UWP)
  endif(CMAKE_SYSTEM_NAME MATCHES WindowsPhone OR CMAKE_SYSTEM_NAME MATCHES WindowsStore)
endif(WIN32)

# rpath handling: use rpath in installed binaries
if(NOT CMAKE_SYSTEM_NAME MATCHES Linux)
  set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

# for code completion frameworks
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Targets
ADD_SUBDIRECTORY(lib)


# fixed size integral types

if(HAVE_INTTYPES_H_LIBZIP)
  set(LIBZIP_TYPES_INCLUDE "#if !defined(__STDC_FORMAT_MACROS)
#define __STDC_FORMAT_MACROS 1
#endif
#include <inttypes.h>")
elseif(HAVE_STDINT_H_LIBZIP)
  set(LIBZIP_TYPES_INCLUDE "#include <stdint.h>")
elseif(HAVE_SYS_TYPES_H_LIBZIP)
  set(LIBZIP_TYPES_INCLUDE "#include <sys/types.h>")
endif()

if(HAVE_INT8_T_LIBZIP)
  set(ZIP_INT8_T int8_t)
elseif(HAVE___INT8_LIBZIP)
  set(ZIP_INT8_T __int8)
else()
  set(ZIP_INT8_T "signed char")
endif()

if(HAVE_UINT8_T_LIBZIP)
  set(ZIP_UINT8_T uint8_t)
elseif(HAVE___INT8_LIBZIP)
  set(ZIP_UINT8_T "unsigned __int8")
else()
  set(ZIP_UINT8_T "unsigned char")
endif()

if(HAVE_INT16_T_LIBZIP)
  set(ZIP_INT16_T int16_t)
elseif(HAVE___INT16_LIBZIP)
  set(INT16_T_LIBZIP __int16)
elseif(SHORT_LIBZIP EQUAL 2)
  set(INT16_T_LIBZIP short)
endif()

if(HAVE_UINT16_T_LIBZIP)
  set(ZIP_UINT16_T uint16_t)
elseif(HAVE___INT16_LIBZIP)
  set(UINT16_T_LIBZIP "unsigned __int16")
elseif(SHORT_LIBZIP EQUAL 2)
  set(UINT16_T_LIBZIP "unsigned short")
endif()

if(HAVE_INT32_T_LIBZIP)
  set(ZIP_INT32_T int32_t)
elseif(HAVE___INT32_LIBZIP)
  set(ZIP_INT32_T __int32)
elseif(INT_LIBZIP EQUAL 4)
  set(ZIP_INT32_T int)
elseif(LONG_LIBZIP EQUAL 4)
  set(ZIP_INT32_T long)
endif()

if(HAVE_UINT32_T_LIBZIP)
  set(ZIP_UINT32_T uint32_t)
elseif(HAVE___INT32_LIBZIP)
  set(ZIP_UINT32_T "unsigned __int32")
elseif(INT_LIBZIP EQUAL 4)
  set(ZIP_UINT32_T "unsigned int")
elseif(LONG_LIBZIP EQUAL 4)
  set(ZIP_UINT32_T "unsigned long")
endif()

if(HAVE_INT64_T_LIBZIP)
  set(ZIP_INT64_T int64_t)
elseif(HAVE___INT64_LIBZIP)
  set(ZIP_INT64_T __int64)
elseif(LONG_LIBZIP EQUAL 8)
  set(ZIP_INT64_T long)
elseif(LONG_LONG_LIBZIP EQUAL 8)
  set(ZIP_INT64_T "long long")
endif()

if(HAVE_UINT64_T_LIBZIP)
  set(ZIP_UINT64_T uint64_t)
elseif(HAVE___INT64_LIBZIP)
  set(ZIP_UINT64_T "unsigned __int64")
elseif(LONG_LIBZIP EQUAL 8)
  set(ZIP_UINT64_T "unsigned long")
elseif(LONG_LONG_LIBZIP EQUAL 8)
  set(ZIP_UINT64_T "unsigned long long")
endif()

if(HAVE_NULLABLE)
  set(ZIP_NULLABLE_DEFINES)
else()
  set(ZIP_NULLABLE_DEFINES "#define _Nullable
#define _Nonnull")
endif()

# write out config file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake-config.h.in ${PROJECT_BINARY_DIR}/config.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake-zipconf.h.in ${PROJECT_BINARY_DIR}/zipconf.h)

