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
#  - native cross-toolchain for OSX
#  - automatic download for toolchains and utilities (apart from git and cmake)
#  - automatic target environment download (for distributed rootfs image)
#  - configure: board type options (target)
#  - configure: debug/release options
#  - configure: toolchain finding using environment variables
#--------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(linuxdrone)

# Project root and custom cmake modules path
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules/" ${CMAKE_MODULE_PATH})

# Install path for relocatable copy (uner the build directory)
set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/install/usr/local/${PROJECT_NAME})

# Compiler options
add_definitions(-std=c++11 ${XENOMAI_DEFINITIONS})

# Library options (comment off to build static libraries)
set(BUILD_SHARED_LIBS TRUE)

# Project source directories
set(LIB_DIR ${PROJECT_SOURCE_DIR}/libraries)
set(MOD_DIR ${PROJECT_SOURCE_DIR}/modules)
set(SYS_DIR ${PROJECT_SOURCE_DIR}/system)
set(TGT_DIR ${PROJECT_SOURCE_DIR}/targets)

# MongoDB
# Libraries: ${MongoDB_LIBRARIES}
find_package(MongoDB REQUIRED)
include_directories(${INCLUDE_DIRECTORIES} ${MongoDB_INCLUDE_DIRS})

# Boost
# Libraries: ${Boost_LIBRARIES}
find_package(Boost REQUIRED COMPONENTS system filesystem thread program_options)
include_directories(${INCLUDE_DIRECTORIES} ${Boost_INCLUDE_DIRS})
link_directories(${LINK_DIRECTORIES} ${Boost_LIBRARY_DIRS})

# Xenomai libraries
# Libraries: ${XENOMAI_LIBRARY_NATIVE} ${XENOMAI_LIBRARY_XENOMAI}
# Compiler options: ${XENOMAI_DEFINITIONS}
find_package(Xenomai REQUIRED)
include_directories(${INCLUDE_DIRECTORIES} ${XENOMAI_INCLUDE_DIR})

# Link-only system libraries (do not check headers, just link shared objects)
set(CMAKE_THREAD_LIBS_INIT pthread)
set(OPENSSL_LIBRARIES ssl crypto)
set(ZLIB_LIBRARIES z)

# RPATH handling to use relative paths in the install tree like above
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib")

# Project libraries
add_subdirectory(${LIB_DIR}/core)
add_subdirectory(${LIB_DIR}/sdk)
#add_subdirectory(${LIB_DIR}/gcs)

# Main project target
add_subdirectory(${TGT_DIR}/beaglebone) 
