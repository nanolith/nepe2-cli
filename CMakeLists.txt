cmake_minimum_required(VERSION 3.25)
cmake_policy(SET CMP0048 NEW)
PROJECT(nepe2cli VERSION 0.1.0)

string(REPLACE "." ";" NEPE2CLI_VERSION_LIST "${CMAKE_PROJECT_VERSION}")
list(GET NEPE2CLI_VERSION_LIST 0 NEPE2CLI_VERSION_MAJOR)
list(GET NEPE2CLI_VERSION_LIST 1 NEPE2CLI_VERSION_MINOR)
list(GET NEPE2CLI_VERSION_LIST 2 NEPE2CLI_VERSION_REL)

cmake_minimum_required(VERSION 3.10)

INCLUDE(CheckSymbolExists)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

option(MODELCHECK_REQUIRED "Require Model Checking")

#pkgconfig
find_package(PkgConfig REQUIRED)
#nepe2base package
pkg_search_module(nepe2base REQUIRED IMPORTED_TARGET nepe2base)
#threads package
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)

#Build config.h
configure_file(config.h.cmake include/nepe2cli/config.h)

#includes
INCLUDE_DIRECTORIES(include)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/include)

#source files
AUX_SOURCE_DIRECTORY(src NEPE2CLI_SOURCES)

ADD_EXECUTABLE(nepe2 ${NEPE2CLI_SOURCES})

TARGET_COMPILE_OPTIONS(
    nepe2 PRIVATE -O2 ${nepe2base_CFLAGS}
                     -Wall -Werror -Wextra -Wpedantic
                     -Wno-unused-command-line-argument)
TARGET_LINK_LIBRARIES(
    nepe2
    PRIVATE -O2 PkgConfig::nepe2base)

#Build a pkg-config file
SET(NEPE2CLI_PC "${CMAKE_BINARY_DIR}/nepe2base.pc")
FILE(WRITE  ${NEPE2CLI_PC} "Name: nepe2base")
FILE(APPEND ${NEPE2CLI_PC} "\nDescription: nepephemeral 2 base library")
FILE(APPEND ${NEPE2CLI_PC} "\nVersion: ${CMAKE_PROJECT_VERSION}")
FILE(APPEND ${NEPE2CLI_PC} "\nURL: https://github.com/nanolith/nepe2-base")
FILE(APPEND ${NEPE2CLI_PC} "\nRequires: rcpr >= 0.4.4, libcrypto >= 2.0.0")
FILE(APPEND ${NEPE2CLI_PC} "\nprefix=${CMAKE_INSTALL_PREFIX}")
FILE(APPEND ${NEPE2CLI_PC} "\nlibdir=\${prefix}/lib")
FILE(APPEND ${NEPE2CLI_PC} "\nincludedir=\${prefix}/include")
FILE(APPEND ${NEPE2CLI_PC} "\nLibs: -L\${libdir} -lnepe2base")
FILE(APPEND ${NEPE2CLI_PC} "\nCflags: -I\${includedir}")
INSTALL(FILES ${NEPE2CLI_PC} DESTINATION lib/pkgconfig)

#Install headers
FILE(GLOB NEPE2CLI_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/include/nepe2/*.h")
INSTALL(FILES ${NEPE2CLI_INCLUDES} DESTINATION include/nepe2)
INSTALL(
    FILES ${CMAKE_BINARY_DIR}/include/nepe2/config.h DESTINATION include/nepe2)

#Install library
INSTALL(TARGETS nepe2
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
