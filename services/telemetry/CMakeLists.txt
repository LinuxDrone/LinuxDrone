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
    ${RFS_DIR}/usr/local/lib/libwebsockets.so
    z
    ssl
    crypto
    rt
    dl
    pthread
)

add_executable(telemetry ${INC} ${SRC})
target_link_libraries(telemetry ${EXTRA_LIBS})

IF(${DO_POST_BUILD})
    add_custom_command(
        TARGET telemetry
        POST_BUILD
    #COMMAND
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} telemetry ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/services/telemetry
    #    COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} ${WEB_DIR}/configurator/public/javascripts/configurator.js ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/webapps/configurator/public/javascripts/configurator.js
    #    COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} ${WEB_DIR}/configurator/public/index.html ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/webapps/configurator/public/index.html
    )
ENDIF()


install(TARGETS telemetry DESTINATION bin)
