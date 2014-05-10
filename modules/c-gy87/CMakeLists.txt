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

include_directories(${LIB_DIR}/c-sdk/include)
include_directories(${RFS_DIR}/usr/local/include/libbson-1.0)

set(EXTRA_LIBS
    c-sdk
)

add_library(c-gy87 MODULE ${INC} ${SRC})

target_link_libraries(c-gy87 ${EXTRA_LIBS})

add_custom_command(
    TARGET c-gy87
    PRE_BUILD
    COMMAND ${NODEJS} ${LIB_DIR}/c-sdk/ModuleGenerator.js ${MOD_DIR}/c-gy87/c-gy87.def.json ${MOD_DIR}/c-gy87
)

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
