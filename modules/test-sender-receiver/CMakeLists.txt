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

#ADD_CUSTOM_COMMAND(
#    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test-sender-receiver.helper.c
#    COMMAND ${NODEJS} ${LIB_DIR}/c-sdk/ModuleGenerator.js ${CMAKE_CURRENT_SOURCE_DIR}/test-sender-receiver.def.json ${CMAKE_CURRENT_BINARY_DIR}
#    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test-sender-receiver.def.json
#)

add_library(test-sender-receiver MODULE ${INC} ${SRC} ${CMAKE_CURRENT_BINARY_DIR}/test-sender-receiver.helper.c)

target_link_libraries(test-sender-receiver ${EXTRA_LIBS})

IF(${DO_POST_BUILD})
    add_custom_command(
        TARGET test-sender-receiver
        POST_BUILD
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} libtest-sender-receiver.so ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/test-sender-receiver/libtest-sender-receiver.so
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} ${MOD_DIR}/test-sender-receiver/test-sender-receiver.def.json ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/test-sender-receiver/test-sender-receiver.def.json
    )
ENDIF()

install(TARGETS test-sender-receiver DESTINATION modules/test-sender-receiver)
install(FILES test-sender-receiver.def.json DESTINATION modules/test-sender-receiver)
