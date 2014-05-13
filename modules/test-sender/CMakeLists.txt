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
include_directories(${RFS_DIR}/usr/local/include/libbson-1.0)

set(EXTRA_LIBS
    c-sdk
)

ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test-sender.helper.c
    COMMAND ${NODEJS} ${LIB_DIR}/c-sdk/ModuleGenerator.js ${CMAKE_CURRENT_SOURCE_DIR}/test-sender.def.json ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test-sender.def.json
)

add_library(test-sender MODULE ${INC} ${SRC} test-sender.helper.c)

target_link_libraries(test-sender ${EXTRA_LIBS})

IF(${DO_POST_BUILD})
    add_custom_command(
        TARGET test-sender
        POST_BUILD
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} libtest-sender.so ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/test-sender/libtest-sender.so
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} ${MOD_DIR}/test-sender/test-sender.def.json ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/test-sender/test-sender.def.json
    )
ENDIF()

install(TARGETS test-sender DESTINATION modules/test-sender)
install(FILES test-sender.def.json DESTINATION modules/test-sender)
