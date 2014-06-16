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

file(GLOB_RECURSE INC "*.h")
file(GLOB_RECURSE SRC "*.c")

include_directories(${RFS_DIR}/usr/local/include/libbson-1.0)
include_directories(${RFS_DIR}/usr/local/include/libmongoc-1.0)
include_directories(${LIB_DIR}/sdk/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../../libraries/sdk)

set(EXTRA_LIBS
    sdk
    ${XENOMAI_LIBRARY_NATIVE} ${XENOMAI_LIBRARY_XENOMAI}
    rt
    pthread
)

add_executable(i2c_service ${INC} ${SRC})
target_link_libraries(i2c_service ${EXTRA_LIBS})

IF(${DO_POST_BUILD})
    add_custom_command(
        TARGET i2c_service
        POST_BUILD
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} i2c_service ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/services/i2c_service
    )
ENDIF()

install(TARGETS i2c_service DESTINATION services)
