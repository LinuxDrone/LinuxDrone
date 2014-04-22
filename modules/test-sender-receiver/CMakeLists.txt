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

add_library(test-sender-receiver MODULE ${INC} ${SRC})

target_link_libraries(test-sender-receiver ${EXTRA_LIBS})

add_custom_command(
    TARGET test-sender-receiver
    PRE_BUILD
COMMAND
    #COMMAND ${NODEJS} ${LIB_DIR}/c-sdk/ModuleGenerator.js ${MOD_DIR}/test-sender-receiver/test-sender-receiver.def.json ${MOD_DIR}/test-sender-receiver
)

add_custom_command(
    TARGET test-sender-receiver
    POST_BUILD
COMMAND
    #COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} libtest-sender-receiver.so ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/test-sender-receiver/libtest-sender-receiver.so
    #COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} ${MOD_DIR}/test-sender-receiver/test-sender-receiver.def.json ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/test-sender-receiver/test-sender-receiver.def.json
)

install(TARGETS test-sender-receiver DESTINATION modules/test-sender-receiver)
install(FILES test-sender-receiver.def.json DESTINATION modules/test-sender-receiver)
