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

#
# BeagleBone Black, Ubuntu 13.10, gcc-linaro-arm-linux-gnueabihf-4.8-2013.10
#

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)

# Specify the cross compiler
if(CMAKE_HOST_WIN32)
    set(CMAKE_HOST_EXECUTABLE_SUFFIX ".exe")
else()
    set(CMAKE_HOST_EXECUTABLE_SUFFIX "")
endif()

set(TOOLCHAIN_PREFIX   ${PROJECT_SOURCE_DIR}/tools/gcc-linaro-arm-linux-gnueabihf-4.8-2013.10/bin/arm-linux-gnueabihf-)
set(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}gcc${CMAKE_HOST_EXECUTABLE_SUFFIX})
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++${CMAKE_HOST_EXECUTABLE_SUFFIX})

# Where is the target environment
set(BOARD_ROOTFS ${PROJECT_SOURCE_DIR}/tools/rootfs/beaglebone)
set(CMAKE_FIND_ROOT_PATH  ${BOARD_ROOTFS})

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# For libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
