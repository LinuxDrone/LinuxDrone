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

include_directories(${LIB_DIR}/sdk/include ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${BSON_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../libraries/sdk)

set(EXTRA_LIBS
    sdk
)

ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/short_terminator.helper.c
    COMMAND ${NODEJS} ${LIB_DIR}/sdk/ModuleGenerator.js ${CMAKE_CURRENT_SOURCE_DIR}/short_terminator.def.json ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/short_terminator.def.json
)

add_library(short_terminator MODULE ${INC} ${SRC} short_terminator.helper.c)

target_link_libraries(short_terminator ${EXTRA_LIBS})

IF(${DO_POST_BUILD})
    add_custom_command(
        TARGET short_terminator
        POST_BUILD
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} libshort_terminator.so ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/short_terminator/libshort_terminator.so
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} ${MOD_DIR}/short_terminator/short_terminator.def.json ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/short_terminator/short_terminator.def.json
    )
ENDIF()

install(TARGETS short_terminator DESTINATION modules/short_terminator)
install(FILES short_terminator.def.json DESTINATION modules/short_terminator)
