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
    m
    pthread
    rt
    dl
)

ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/c-scale.helper.c
    COMMAND ${NODEJS} ${LIB_DIR}/sdk/ModuleGenerator.js ${CMAKE_CURRENT_SOURCE_DIR}/c-scale.def.json ${CMAKE_CURRENT_BINARY_DIR} ${PLATFORM}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/c-scale.def.json
)

add_executable(c-scale ${INC} ${SRC} c-scale.helper.c)

target_link_libraries(c-scale ${EXTRA_LIBS})

IF(${DO_POST_BUILD})
    add_custom_command(
        TARGET c-scale
        POST_BUILD
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} c-scale ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/bin/c-scale.mod
    )
ENDIF()

install(TARGETS c-scale DESTINATION bin)
