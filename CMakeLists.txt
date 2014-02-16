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

# Install path for relocatable copy (uner the build directory)
set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/install/usr/local/${PROJECT_NAME})

# Compiler options
add_definitions(-std=c++11 -mfpu=neon ${XENOMAI_DEFINITIONS})

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

# Xenomai libraries
# Libraries: ${XENOMAI_LIBRARY_NATIVE} ${XENOMAI_LIBRARY_XENOMAI}
# Compiler options: ${XENOMAI_DEFINITIONS}
find_package(Xenomai REQUIRED)
include_directories(${INCLUDE_DIRECTORIES} ${XENOMAI_INCLUDE_DIR})

# Threads
find_package(Threads)

# Link-only libraries (do not check headers, just link shared objects)
find_library(ZLIB_LIBRARIES z)
find_library(OPENSSL_LIBRARIES ssl)
find_library(CRYPTO_LIBRARIES crypto)

# RPATH handling to use relative paths in the install tree
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

# Project libraries
set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib")
add_subdirectory(${LIB_DIR}/core)
add_subdirectory(${LIB_DIR}/sdk)

# Loadable modules (one level deeper than apps or libs)
set(CMAKE_INSTALL_RPATH "\$ORIGIN/../../lib")
add_subdirectory(${MOD_DIR}/gy87)
add_subdirectory(${MOD_DIR}/bmp180)
add_subdirectory(${MOD_DIR}/ms5611)
add_subdirectory(${MOD_DIR}/hmc5883)
add_subdirectory(${MOD_DIR}/pwmoutput)
add_subdirectory(${MOD_DIR}/PwmInput)
add_subdirectory(${MOD_DIR}/SimpleTransform)

# Main project target
set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib")
add_subdirectory(${TGT_DIR}/beaglebone) 
