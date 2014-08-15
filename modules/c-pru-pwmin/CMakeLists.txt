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
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/c-pru-pwmin.helper.c
    COMMAND ${NODEJS} ${LIB_DIR}/sdk/ModuleGenerator.js ${CMAKE_CURRENT_SOURCE_DIR}/c-pru-pwmin.def.json ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/c-pru-pwmin.def.json
)

add_library(c-pru-pwmin MODULE ${INC} ${SRC} c-pru-pwmin.helper.c)

ADD_CUSTOM_COMMAND(
    TARGET c-pru-pwmin
    COMMAND ${PASM} -V3 -b ${CMAKE_CURRENT_SOURCE_DIR}/pru-pwmin.p ${CMAKE_CURRENT_BINARY_DIR}/pru-pwmin
    COMMAND ${DTC} -I dts -O dtb ${CMAKE_CURRENT_SOURCE_DIR}/BB-BONE-PRU-LD-00A0.dts > ${CMAKE_CURRENT_BINARY_DIR}/BB-BONE-PRU-LD-00A0.dtbo
)

target_link_libraries(c-pru-pwmin ${EXTRA_LIBS})

IF(${DO_POST_BUILD})
    add_custom_command(
        TARGET c-pru-pwmin
        POST_BUILD
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} BB-BONE-PRU-LD-00A0.dtbo ${URL_TARGET_SYSTEM}:/lib/firmware/BB-BONE-PRU-LD-00A0.dtbo
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} pru-pwmin.bin ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/c-pru-pwmin/pru-pwmin.bin
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} libc-pru-pwmin.so ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/c-pru-pwmin/libc-pru-pwmin.so
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} ${MOD_DIR}/c-pru-pwmin/c-pru-pwmin.def.json ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/c-pru-pwmin/c-pru-pwmin.def.json
    )
ENDIF()

install(TARGETS c-pru-pwmin DESTINATION modules/c-pru-pwmin)
install(FILES c-pru-pwmin.def.json DESTINATION modules/c-pru-pwmin)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/pru-pwmin.bin DESTINATION modules/c-pru-pwmin)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/BB-BONE-PRU-LD-00A0.dtbo DESTINATION ../../../lib/firmware)
