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

IF(${PLATFORM} STREQUAL "XENO")
    set(EXTRA_LIBS
        sdk
        pthread
        rt
    )
ELSE()
    set(EXTRA_LIBS
        sdk
    )
ENDIF()


ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test-sender.helper.c
    COMMAND ${NODEJS} ${LIB_DIR}/sdk/ModuleGenerator.js ${CMAKE_CURRENT_SOURCE_DIR}/test-sender.def.json ${CMAKE_CURRENT_BINARY_DIR} ${PLATFORM}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test-sender.def.json
)

add_executable(test-sender ${INC} ${SRC} test-sender.helper.c)

target_link_libraries(test-sender ${EXTRA_LIBS})

IF(${DO_POST_BUILD})
    add_custom_command(
        TARGET test-sender
        POST_BUILD
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} test-sender ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/bin/test-sender.mod
    )
ENDIF()

install(TARGETS test-sender DESTINATION bin)
