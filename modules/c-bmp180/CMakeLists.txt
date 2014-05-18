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
    i2c_client
    m
)

ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/c-bmp180.helper.c
    COMMAND ${NODEJS} ${LIB_DIR}/c-sdk/ModuleGenerator.js ${CMAKE_CURRENT_SOURCE_DIR}/c-bmp180.def.json ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/c-bmp180.def.json
)

add_library(c-bmp180 MODULE ${INC} ${SRC} c-bmp180.helper.c)

target_link_libraries(c-bmp180 ${EXTRA_LIBS})

IF(${DO_POST_BUILD})
    add_custom_command(
        TARGET c-bmp180
        POST_BUILD
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} libc-bmp180.so ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/c-bmp180/libc-bmp180.so
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} ${MOD_DIR}/c-bmp180/c-bmp180.def.json ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/c-bmp180/c-bmp180.def.json
    )
ENDIF()

install(TARGETS c-bmp180 DESTINATION lib)
install(FILES c-bmp180.def.json DESTINATION modules/c-bmp180)
