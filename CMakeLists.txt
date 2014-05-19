#--------------------------------------------------------------------
# This file was created as a part of the LinuxDrone project:
#                http://www.linuxdrone.org
#
# Distributed under the Creative Commons Attribution-ShareAlike 4.0
# International License (see accompanying License.txt file or a copy
# at http://creativecommons.org/licenses/by-sa/4.0/legalcode)
#
# The human-readable summary of (and not a substitute for) the
# license: http://creativecommons.org/licenses/by-sa/4.0/
#--------------------------------------------------------------------

#--------------------------------------------------------------------
# TODO:
#  - automatic download for toolchains and utilities (apart from git and cmake)
#  - automatic target environment download (for distributed rootfs image)
#  - compiler options (debug/release and others)
#  - configure: board type options (target)
#  - configure: debug/release options
#  - add javascript code install target
#--------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project(linuxdrone)

# Extra cmake modules path (must be set by toolchain file passed from command line)
#set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules/" ${CMAKE_MODULE_PATH})

# Install path for relocatable copy (under the build directory)
set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/install/usr/local/${PROJECT_NAME})

# Compiler options
add_definitions(-mfpu=neon ${XENOMAI_DEFINITIONS}) #-std=c++11

# Library options (comment off to build static libraries)
set(BUILD_SHARED_LIBS TRUE)

# Project source directories
set(LIB_DIR ${PROJECT_SOURCE_DIR}/libraries)
set(MOD_DIR ${PROJECT_SOURCE_DIR}/modules)
set(HST_DIR ${PROJECT_SOURCE_DIR}/host-programs)
set(SVC_DIR ${PROJECT_SOURCE_DIR}/services)
set(SYS_DIR ${PROJECT_SOURCE_DIR}/system)
set(TGT_DIR ${PROJECT_SOURCE_DIR}/targets)
set(TLS_DIR ${PROJECT_SOURCE_DIR}/tools)
set(WEB_DIR ${PROJECT_SOURCE_DIR}/webapps)
set(RFS_DIR ${TLS_DIR}/rootfs/beaglebone)


IF(${CMAKE_HOST_SYSTEM} MATCHES "Windows")
    SET(SCP ${TLS_DIR}/pscp.exe)
    SET(NODEJS node)
    SET(PASM ${TLS_DIR}/pasm/pasm.exe)
ELSEIF(${CMAKE_HOST_SYSTEM} MATCHES "Linux")
    SET(SCP scp)
    SET(NODEJS node)
    SET(PASM ${TLS_DIR}/pasm/pasm)
ELSE()
    SET(SCP scp)
    SET(NODEJS /opt/local/bin/node)
    SET(PASM ${TLS_DIR}/pasm/pasm)
ENDIF()

SET(DO_POST_BUILD YES)

#SET(URL_TARGET_SYSTEM root@vrubel.linuxdrone.org)
#SET(URL_TARGET_SYSTEM root@192.168.1.2)
#SET(SSH_PORT_TARGET_SYSTEM 22)

#SET(URL_TARGET_SYSTEM root@voha-bbb.linuxdrone.org)
#SET(SSH_PORT_TARGET_SYSTEM 1234)

SET(URL_TARGET_SYSTEM root@192.168.1.43)
SET(SSH_PORT_TARGET_SYSTEM 22)

# MongoDB
# Libraries: ${MongoDB_LIBRARIES}
find_package(MongoDB REQUIRED)
include_directories(${INCLUDE_DIRECTORIES} ${MongoDB_INCLUDE_DIRS})

# Boost
# Libraries: ${Boost_LIBRARIES}
set(Boost_USE_STATIC_LIBS OFF) 
find_package(Boost REQUIRED COMPONENTS system filesystem thread program_options)
include_directories(${INCLUDE_DIRECTORIES} ${Boost_INCLUDE_DIRS})

# Xenomai libraries
# Libraries: ${XENOMAI_LIBRARY_NATIVE} ${XENOMAI_LIBRARY_XENOMAI}
# Compiler options: ${XENOMAI_DEFINITIONS}
find_package(Xenomai REQUIRED)
include_directories(${INCLUDE_DIRECTORIES} ${XENOMAI_INCLUDE_DIR})

# Threads
find_package(Threads)


#option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" OFF)
IF(BUILD_DOCUMENTATION)

    FIND_PACKAGE(Doxygen)
    IF(NOT DOXYGEN_FOUND)
        MESSAGE(FATAL_ERROR "Doxygen is needed to build the documentation.")
    ENDIF()

    SET( doxyfile_in      ${PROJECT_SOURCE_DIR}/Doxyfile.in)
    SET( DOXY_INPUT       ${PROJECT_SOURCE_DIR})        # Pasted into Doxyfile

    SET( doxyfile_ru      ${PROJECT_BINARY_DIR}/Doxyfile.ru)
    SET( doxy_ru_index    ${PROJECT_BINARY_DIR}/ru/html/index.html)
    SET( DOXY_OUTPUT_DIR  ${PROJECT_BINARY_DIR}/ru)     # Pasted into Doxyfile.ru
    SET( DOXY_OUT_LANG    Russian)                      # Pasted into Doxyfile.ru
    CONFIGURE_FILE(${doxyfile_in} ${doxyfile_ru} @ONLY IMMEDIATE)
    ADD_CUSTOM_COMMAND(
        OUTPUT ${doxy_ru_index}
        COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile.ru
        MAIN_DEPENDENCY ${doxyfile_ru} ${doxyfile_in}
        DEPENDS c-sdk ${doxy_extra_files}
        COMMENT "Generating RU HTML documentation"
    )


    SET( doxyfile_en      ${PROJECT_BINARY_DIR}/Doxyfile.en)
    SET( doxy_en_index    ${PROJECT_BINARY_DIR}/en/html/index.html)
    SET( DOXY_OUTPUT_DIR  ${PROJECT_BINARY_DIR}/en)     # Pasted into Doxyfile.en
    SET( DOXY_OUT_LANG    English)                      # Pasted into Doxyfile.en
    CONFIGURE_FILE(${doxyfile_in} ${doxyfile_en} @ONLY IMMEDIATE)
    ADD_CUSTOM_COMMAND(
        OUTPUT ${doxy_en_index}
        COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile.en
        MAIN_DEPENDENCY ${doxyfile_en} ${doxyfile_in}
        DEPENDS c-sdk ${doxy_extra_files}
        COMMENT "Generating EN HTML documentation"
    )


    ADD_CUSTOM_TARGET( doc ALL DEPENDS ${doxy_ru_index} ${doxy_en_index})

    INSTALL( DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc )
ENDIF()



# Link-only libraries (do not check headers, just link shared objects)
find_library(ZLIB_LIBRARIES z)
find_library(OPENSSL_LIBRARIES ssl)
find_library(CRYPTO_LIBRARIES crypto)

# RPATH handling to use relative paths in the install tree
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

# Project libraries
set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib")
#add_subdirectory(${LIB_DIR}/core)
#add_subdirectory(${LIB_DIR}/sdk)
#add_subdirectory(${LIB_DIR}/dotnet-sdk)
add_subdirectory(${LIB_DIR}/c-sdk)

# Loadable modules (one level deeper than apps or libs)
set(CMAKE_INSTALL_RPATH "\$ORIGIN/../../lib")
#add_subdirectory(${MOD_DIR}/gy87)
add_subdirectory(${MOD_DIR}/rx-openlrs)
add_subdirectory(${MOD_DIR}/short_terminator)
add_subdirectory(${MOD_DIR}/c-gy87)
#add_subdirectory(${MOD_DIR}/test-params)
add_subdirectory(${MOD_DIR}/test-sender)
add_subdirectory(${MOD_DIR}/test-receiver)
add_subdirectory(${MOD_DIR}/test-sender-receiver)
add_subdirectory(${MOD_DIR}/c-pru-pwmout)
#add_subdirectory(${MOD_DIR}/MyModule)
#add_subdirectory(${MOD_DIR}/bmp180)
#add_subdirectory(${MOD_DIR}/ms5611)
#add_subdirectory(${MOD_DIR}/hmc5883)
#add_subdirectory(${MOD_DIR}/pwmoutput)
#add_subdirectory(${MOD_DIR}/PwmInput)
#add_subdirectory(${MOD_DIR}/SimpleTransform)
#add_subdirectory(${MOD_DIR}/SerialGps)
add_subdirectory(${MOD_DIR}/c-bmp180)

# Host programms
add_subdirectory(${HST_DIR}/c-host)

# Services
add_subdirectory(${SVC_DIR}/telemetry)
add_subdirectory(${SVC_DIR}/i2c)

# Main project target
set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib")
#add_subdirectory(${TGT_DIR}/beaglebone) 
