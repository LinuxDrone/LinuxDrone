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
include_directories(${RFS_DIR}/usr/local/include/libmongoc-1.0)
include_directories(${LIB_DIR}/c-sdk/include)

set(EXTRA_LIBS
#    c-sdk
    ${XENOMAI_LIBRARY_NATIVE} ${XENOMAI_LIBRARY_XENOMAI}
#    ${RFS_DIR}/usr/local/lib/libbson-1.0.so
#    ${RFS_DIR}/usr/local/lib/libmongoc-1.0.so
#    ${RFS_DIR}/usr/local/lib/libwebsockets.so
#    z
#    ssl
#    crypto
#    rt
#    dl
#    pthread
)

add_executable(i2c ${INC} ${SRC})
target_link_libraries(i2c ${EXTRA_LIBS})

add_custom_command(
    TARGET i2c
    POST_BUILD
#COMMAND
    COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} i2c ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/services/i2c
)


install(TARGETS i2c DESTINATION bin)
