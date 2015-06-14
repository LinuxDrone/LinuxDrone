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

add_definitions(-DCONFIG_HAL_BOARD=HAL_BOARD_LINUX)
add_definitions(-DCONFIG_HAL_BOARD_SUBTYPE=HAL_BOARD_SUBTYPE_LINUX_ERLE)
add_definitions(-DSKETCHNAME="ArduCopter")

include_directories(${LIB_DIR}/sdk/include ${CMAKE_CURRENT_BINARY_DIR})
include_directories(
    ${BSON_INCLUDE_DIR}
    ../../libraries/ardu/ardupilot/libraries/AP_Common
    ../../libraries/ardu/ardupilot/libraries/AP_Math
    ../../libraries/ardu/ardupilot/libraries/AP_Param
    ../../libraries/ardu/ardupilot/libraries/AP_HAL
    ../../libraries/ardu/ardupilot/libraries/AP_Progmem
    ../../libraries/ardu/ardupilot/libraries/RC_Channel
    ../../libraries/ardu/ardupilot/libraries/AP_Notify
    ../../libraries/ardu/ardupilot/libraries/AP_Curve
    ../../libraries/ardu/ardupilot/libraries/AP_Motors
    ../../libraries/ardu/ardupilot/libraries/GCS_MAVLink
    ../../libraries/ardu/ardupilot/libraries/Filter
)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/../../libraries/sdk)
set(EXTRA_LIBS
    sdk
)

ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ap-motors-quad.helper.c
    COMMAND ${NODEJS} ${LIB_DIR}/sdk/ModuleGenerator.js ${CMAKE_CURRENT_SOURCE_DIR}/ap-motors-quad.def.json ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ap-motors-quad.def.json
)

add_library(ap-motors-quad MODULE ${INC} ${SRC} ap-motors-quad.helper.c)

target_link_libraries(ap-motors-quad ${EXTRA_LIBS})

IF(${DO_POST_BUILD})
    add_custom_command(
        TARGET ap-motors-quad
        POST_BUILD
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} libap-motors-quad.so ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/ap-motors-quad/libap-motors-quad.so
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} ${MOD_DIR}/ap-motors-quad/ap-motors-quad.def.json ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/ap-motors-quad/ap-motors-quad.def.json
    )
ENDIF()

install(TARGETS ap-motors-quad DESTINATION modules/ap-motors-quad)
install(FILES ap-motors-quad.def.json DESTINATION modules/ap-motors-quad)
