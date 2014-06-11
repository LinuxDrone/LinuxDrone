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
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../libraries/c-sdk)

set(EXTRA_LIBS
    c-sdk
    m
)

ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/c-sum-scale.helper.c
    COMMAND ${NODEJS} ${LIB_DIR}/c-sdk/ModuleGenerator.js ${CMAKE_CURRENT_SOURCE_DIR}/c-sum-scale.def.json ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/c-sum-scale.def.json
)

add_library(c-sum-scale MODULE ${INC} ${SRC} c-sum-scale.helper.c)

target_link_libraries(c-sum-scale ${EXTRA_LIBS})

IF(${DO_POST_BUILD})
    add_custom_command(
        TARGET c-sum-scale
        POST_BUILD
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} libc-sum-scale.so ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/c-sum-scale/libc-sum-scale.so
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} ${MOD_DIR}/c-sum-scale/c-sum-scale.def.json ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/c-sum-scale/c-sum-scale.def.json
    )
ENDIF()

install(TARGETS c-sum-scale DESTINATION modules/c-sum-scale)
install(FILES c-sum-scale.def.json DESTINATION modules/c-sum-scale)
