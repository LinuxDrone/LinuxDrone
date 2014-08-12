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

include_directories(${BSON_INCLUDE_DIR})
include_directories(${MongoC_INCLUDE_DIR})
include_directories(${LIB_DIR}/c-sdk/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../../libraries/c-sdk)

set(EXTRA_LIBS
    c-sdk
    ${XENOMAI_LIBRARY_NATIVE} ${XENOMAI_LIBRARY_XENOMAI}
    rt
    pthread
)

add_library(spi_client ${INC} ${SRC})
target_link_libraries(spi_client ${EXTRA_LIBS})

IF(${DO_POST_BUILD})
    add_custom_command(
        TARGET spi_client
        POST_BUILD
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} libspi_client.so ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/lib/libspi_client.so
    )
ENDIF()

install(TARGETS spi_client DESTINATION lib)
