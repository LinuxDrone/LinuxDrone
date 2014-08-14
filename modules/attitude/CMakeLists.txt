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
file(GLOB_RECURSE SRC "*.c" "*.cpp")

include_directories(${LIB_DIR}/sdk/include ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${RFS_DIR}/usr/local/include/libbson-1.0)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../libraries/sdk)

set(EXTRA_LIBS
    sdk
    m
)

ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/attitude.helper.c
    COMMAND ${NODEJS} ${LIB_DIR}/sdk/ModuleGenerator.js ${CMAKE_CURRENT_SOURCE_DIR}/attitude.def.json ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/attitude.def.json
)

add_library(attitude MODULE ${INC} ${SRC} attitude.helper.c)

target_link_libraries(attitude ${EXTRA_LIBS})

IF(${DO_POST_BUILD})
    add_custom_command(
        TARGET attitude
        POST_BUILD
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} libattitude.so ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/attitude/libattitude.so
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} ${MOD_DIR}/attitude/attitude.def.json ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/attitude/attitude.def.json
    )
ENDIF()

install(TARGETS attitude DESTINATION modules/attitude)
install(FILES attitude.def.json DESTINATION modules/attitude)
