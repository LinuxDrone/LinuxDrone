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
    c-sdk
    ${XENOMAI_LIBRARY_NATIVE} ${XENOMAI_LIBRARY_XENOMAI}
    ${RFS_DIR}/usr/local/lib/libbson-1.0.so
    ${RFS_DIR}/usr/local/lib/libmongoc-1.0.so
    rt
    dl
    pthread
)

add_executable(c-host ${INC} ${SRC})
target_link_libraries(c-host ${EXTRA_LIBS})

add_custom_command(
    TARGET c-host
    POST_BUILD
#COMMAND
    COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} c-host ${URL_TARGET_SYSTEM}:/root/c-host
    #COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} ${HST_DIR}/c-host/testConfig.json ${URL_TARGET_SYSTEM}:/root/testConfig.json
)


install(TARGETS c-host DESTINATION bin)
