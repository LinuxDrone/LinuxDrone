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

include_directories(${LIB_DIR}/c-sdk/include ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${BSON_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../libraries/c-sdk)

set(EXTRA_LIBS
    c-sdk
    i2c_client
)

ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/rx-openlrs.helper.c
    COMMAND ${NODEJS} ${LIB_DIR}/c-sdk/ModuleGenerator.js ${CMAKE_CURRENT_SOURCE_DIR}/rx-openlrs.def.json ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/rx-openlrs.def.json
)

add_library(rx-openlrs MODULE ${INC} ${SRC} rx-openlrs.helper.c)

target_link_libraries(rx-openlrs ${EXTRA_LIBS})

IF(${DO_POST_BUILD})
    add_custom_command(
        TARGET rx-openlrs
        POST_BUILD
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} librx-openlrs.so ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/rx-openlrs/librx-openlrs.so
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} ${MOD_DIR}/rx-openlrs/rx-openlrs.def.json ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/rx-openlrs/rx-openlrs.def.json
    )
ENDIF()

install(TARGETS rx-openlrs DESTINATION modules/rx-openlrs)
install(FILES rx-openlrs.def.json DESTINATION modules/rx-openlrs)
