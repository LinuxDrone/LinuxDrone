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

file(GLOB_RECURSE INC "include/*")
file(GLOB_RECURSE SRC "src/*.c")

include_directories(${BSON_INCLUDE_DIR})
include_directories(${MongoC_INCLUDE_DIR})
include_directories(${LIB_DIR}/c-sdk/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../libraries/c-sdk)

set(EXTRA_LIBS
    c-sdk
    ${XENOMAI_LIBRARY_NATIVE} ${XENOMAI_LIBRARY_XENOMAI}
    ${BSON_LIBRARIES}
    ${MongoC_LIBRARIES}
    rt
    dl
    pthread
)

add_executable(c-host ${INC} ${SRC})
target_link_libraries(c-host ${EXTRA_LIBS})


IF(${DO_POST_BUILD})
    add_custom_command(
        TARGET c-host
        POST_BUILD
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} c-host ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/bin/c-host
    )
ENDIF()


install(TARGETS c-host DESTINATION bin)
