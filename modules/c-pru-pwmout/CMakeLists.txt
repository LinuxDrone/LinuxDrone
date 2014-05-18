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
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/c-pru-pwmout.helper.c
    COMMAND ${NODEJS} ${LIB_DIR}/c-sdk/ModuleGenerator.js ${CMAKE_CURRENT_SOURCE_DIR}/c-pru-pwmout.def.json ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/c-pru-pwmout.def.json
)

add_library(c-pru-pwmout MODULE ${INC} ${SRC} c-pru-pwmout.helper.c)

ADD_CUSTOM_COMMAND(
    TARGET c-pru-pwmout
    COMMAND ${PASM} -V3 -b ${CMAKE_CURRENT_SOURCE_DIR}/pru-pwmout.p ${CMAKE_CURRENT_BINARY_DIR}/pru-pwmout
)

target_link_libraries(c-pru-pwmout ${EXTRA_LIBS})

IF(${DO_POST_BUILD})
    add_custom_command(
        TARGET c-pru-pwmout
        POST_BUILD
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} pru-pwmout.bin ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/c-pru-pwmout/pru-pwmout.bin
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} libc-pru-pwmout.so ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/c-pru-pwmout/libc-pru-pwmout.so
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} ${MOD_DIR}/c-pru-pwmout/c-pru-pwmout.def.json ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/c-pru-pwmout/c-pru-pwmout.def.json
    )
ENDIF()

install(TARGETS c-pru-pwmout DESTINATION modules/c-pru-pwmout)
install(FILES c-pru-pwmout.def.json DESTINATION modules/c-pru-pwmout)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pru-pwmout.bin DESTINATION modules/c-pru-pwmout)
