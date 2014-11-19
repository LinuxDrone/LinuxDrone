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
# It is possible to set LINUXDRONE_TOOLS_DIR environment variable to override
# local tools directory. So the same toolchains can be used for all working
# copies. Particularly useful for CI server build agents, but also for local
# installations. If no such variable defined, cmake will use directory
# ${PROJECT_SOURCE_DIR}/tools/ under the source tree.
#
# Similarly, LINUXDRONE_DOWNLOADS_DIR can be defined instead of default
# ${PROJECT_SOURCE_DIR}/downloads/ directory to download tools distribution
# tarballs or binary archives to install them into TOOLS_DIR.
#
# Finally, LINUXDRONE_ROOTFS_DIR can be defined as an alternative to default
# ${TOOLS_DIR}/rootfs/ which must contain (parts of) the target board root
# filesystem with headers and libraries used for cross compilation.
#

if(DEFINED ENV{LINUXDRONE_TOOLS_DIR})
    file(TO_CMAKE_PATH "$ENV{LINUXDRONE_TOOLS_DIR}" TOOLS_DIR)
    message(STATUS "User-defined tools directory: ${TOOLS_DIR}")
else()
    set(TOOLS_DIR ${PROJECT_SOURCE_DIR}/tools)
    message(STATUS "Automatically set tools directory: ${TOOLS_DIR}")
endif()

if(DEFINED ENV{LINUXDRONE_DOWNLOADS_DIR})
    file(TO_CMAKE_PATH "$ENV{LINUXDRONE_DOWNLOADS_DIR}" DOWNLOADS_DIR)
    message(STATUS "User-defined downloads directory: ${DOWNLOADS_DIR}")
else()
    set(DOWNLOADS_DIR ${PROJECT_SOURCE_DIR}/downloads)
    message(STATUS "Automatically set downloads directory: ${DOWNLOADS_DIR}")
endif()

