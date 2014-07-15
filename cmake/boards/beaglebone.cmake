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

# Include guard
if(__BEAGLEBONE_CMAKE)
    return()
endif()
set(__BEAGLEBONE_CMAKE 1)

#
# BeagleBone Black, Ubuntu 13.10, gcc-linaro-arm-linux-gnueabihf-4.8-2013.10
#

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)

# Extra cmake modules path (must be here since this is the first file which
# is processed by cmake, but it also uses custom modules)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules/" ${CMAKE_MODULE_PATH})

# Get downloads and tools directories (read the module source for details)
include(FindProjectDirectories)

if(CROSS_COMPILED_USE)
	# Define the cross compiler details (as extracted from distribution)
	if(CMAKE_HOST_WIN32)
		set(TOOLCHAIN_SUBDIR "gcc-linaro-arm-linux-gnueabihf-4.8-2013.10_win32")
		set(CMAKE_HOST_EXECUTABLE_SUFFIX ".exe")
	elseif(CMAKE_HOST_APPLE)
		set(TOOLCHAIN_SUBDIR "gcc-linaro-arm-linux-gnueabihf-4.8-2013.10_osx")
		set(CMAKE_HOST_EXECUTABLE_SUFFIX "")
	else()
		set(TOOLCHAIN_SUBDIR "gcc-linaro-arm-linux-gnueabihf-4.8-2013.10_linux")
		set(CMAKE_HOST_EXECUTABLE_SUFFIX "")
	endif()
	set(CROSS_TOOLCHAIN_PREFIX arm-linux-gnueabihf-)

	# Define compilers
	set(TOOLCHAIN_PREFIX   ${TOOLS_DIR}/${TOOLCHAIN_SUBDIR}/bin/${CROSS_TOOLCHAIN_PREFIX})
	set(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}gcc${CMAKE_HOST_EXECUTABLE_SUFFIX})
	set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++${CMAKE_HOST_EXECUTABLE_SUFFIX})
else()
	# Define compilers
	set(CMAKE_C_COMPILER   gcc)
	set(CMAKE_CXX_COMPILER g++)
endif()

# Where is the target environment
set(BOARD_ROOTFS ${ROOTFS_DIR}/beaglebone)
set(CMAKE_FIND_ROOT_PATH  ${BOARD_ROOTFS})

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# For libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
