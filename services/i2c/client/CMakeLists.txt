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
include_directories(${LIB_DIR}/c-sdk/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../../libraries/c-sdk)

set(EXTRA_LIBS
    c-sdk
    ${XENOMAI_LIBRARY_NATIVE} ${XENOMAI_LIBRARY_XENOMAI}
    rt
    pthread
)

add_library(i2c_client ${INC} ${SRC})
target_link_libraries(i2c_client ${EXTRA_LIBS})

IF(${DO_POST_BUILD})
    add_custom_command(
        TARGET i2c_client
        POST_BUILD
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} libi2c_client.so ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/lib/libi2c_client.so
    )
ENDIF()

install(TARGETS i2c_client DESTINATION lib)
