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
IF(${PLATFORM} STREQUAL "XENO")
    SET(INCLUDE_FILES "include/module-functions.h" )
    SET(SRC_FILES "src/helper-functions.c" "src/module-functions.c")
ELSE()
    SET(INCLUDE_FILES "include/apr-module-functions.h" )
    SET(SRC_FILES "src/apr-helper-functions.c" "src/apr-module-functions.c" )
ENDIF()

IF(${PLATFORM} STREQUAL "XENO")
    file(GLOB_RECURSE INC "include/math/*" "include/pru/*" ${INCLUDE_FILES})
    file(GLOB_RECURSE SRC "src/math/*" "src/pru/*" ${SRC_FILES})
ELSE()
    file(GLOB_RECURSE INC ${INCLUDE_FILES})
    file(GLOB_RECURSE SRC ${SRC_FILES})
ENDIF()



include_directories(${RFS_DIR}/usr/local/include/libbson-1.0 ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include)

IF(${PLATFORM} STREQUAL "XENO")
    set(EXTRA_LIBS
        ${XENOMAI_LIBRARY_NATIVE} ${XENOMAI_LIBRARY_XENOMAI}
        ${RFS_DIR}/usr/local/lib/libbson-1.0.so
        ${RFS_DIR}/usr/local/apr/lib/libapr-2.so
        m
        expat
    )
ELSE()
    set(EXTRA_LIBS
        ${RFS_DIR}/libbson/lib/win/x64/bson-1.0.lib
        ${RFS_DIR}/libapr/lib/win/x64/libapr-1.lib
    )
ENDIF()



ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/common-params.c
    COMMAND ${NODEJS} ${LIB_DIR}/sdk/CommonParamsGenerator.js ${CMAKE_CURRENT_SOURCE_DIR}/../../webapps/configurator/client/ModulesCommonParams.def.js ${CMAKE_CURRENT_BINARY_DIR} ${PLATFORM}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../../webapps/configurator/client/ModulesCommonParams.def.js
)

add_library(sdk ${INC} ${SRC} common-params.c)

target_link_libraries(sdk ${EXTRA_LIBS})

IF(${DO_POST_BUILD})
    add_custom_command(
        TARGET sdk
        POST_BUILD
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} libsdk.so ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/lib/libsdk.so
    )
ENDIF()

install(TARGETS sdk DESTINATION lib)
