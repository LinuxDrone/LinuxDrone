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
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/c-gy87.helper.c
    COMMAND ${NODEJS} ${LIB_DIR}/c-sdk/ModuleGenerator.js ${CMAKE_CURRENT_SOURCE_DIR}/c-gy87.def.json ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/c-gy87.def.json
)

add_library(c-gy87 MODULE ${INC} ${SRC} c-gy87.helper.c)

target_link_libraries(c-gy87 ${EXTRA_LIBS})

IF(${DO_POST_BUILD})
    add_custom_command(
        TARGET c-gy87
        POST_BUILD
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} libc-gy87.so ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/c-gy87/libc-gy87.so
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} ${MOD_DIR}/c-gy87/c-gy87.def.json ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/c-gy87/c-gy87.def.json
    )
ENDIF()

install(TARGETS c-gy87 DESTINATION modules/c-gy87)
install(FILES c-gy87.def.json DESTINATION modules/c-gy87)
