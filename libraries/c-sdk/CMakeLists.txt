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

include_directories(${RFS_DIR}/usr/local/include/libbson-1.0)

set(EXTRA_LIBS
    ${XENOMAI_LIBRARY_NATIVE} ${XENOMAI_LIBRARY_XENOMAI}
    ${RFS_DIR}/usr/local/lib/libbson-1.0.so
)

add_library(c-sdk ${INC} ${SRC})

target_link_libraries(c-sdk ${EXTRA_LIBS})

IF(${DO_POST_BUILD})
    add_custom_command(
        TARGET c-sdk
        POST_BUILD
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} libc-sdk.so ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/lib/libc-sdk.so
    )
ENDIF()

install(TARGETS c-sdk DESTINATION lib)
