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
file(GLOB_RECURSE SRC "*.cpp")

include_directories(${LIB_DIR}/sdk/include ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${BSON_INCLUDE_DIR})

include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../libraries/sdk)
set(EXTRA_LIBS
    sdk
    pthread
    rt
)

ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ap-motors-matrix.helper.c
    COMMAND ${NODEJS} ${LIB_DIR}/sdk/ModuleGenerator.js ${CMAKE_CURRENT_SOURCE_DIR}/ap-motors-matrix.def.json ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ap-motors-matrix.def.json
)

add_executable(ap-motors-matrix ${INC} ${SRC} ap-motors-matrix.helper.c)

target_link_libraries(ap-motors-matrix ${EXTRA_LIBS})

IF(${DO_POST_BUILD})
    add_custom_command(
        TARGET ap-motors-matrix
        POST_BUILD
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} ap-motors-matrix ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/bin/ap-motors-matrix
    )
ENDIF()

install(TARGETS ap-motors-matrix DESTINATION bin)
