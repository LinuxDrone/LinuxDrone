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
add_definitions(-DCONFIG_HAL_BOARD=HAL_BOARD_LINUX)
add_definitions(-DCONFIG_HAL_BOARD_SUBTYPE=HAL_BOARD_SUBTYPE_LINUX_ERLE)
add_definitions(-DSKETCHNAME="ArduCopter")

file(GLOB_RECURSE INC "*.h")
file(GLOB_RECURSE SRC "*.cpp")

include_directories(${LIB_DIR}/sdk/include ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${BSON_INCLUDE_DIR})

include_directories(
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_Common
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_HAL
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_Progmem
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_Menu
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_Param
    ${LIB_DIR}/ardu/ardupilot/libraries/StorageManager
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_HAL_AVR
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_HAL_Linux
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_Math
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_Vehicle
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_GPS
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_InertialSensor
    ${LIB_DIR}/ardu/ardupilot/libraries/Filter
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_Baro
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_RangeFinder
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_NavEKF
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_AHRS
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_Declination
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_Compass
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_Airspeed
    ${LIB_DIR}/ardu/ardupilot/libraries/DataFlash
    ${LIB_DIR}/ardu/ardupilot/libraries/GCS_MAVLink
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_Mission
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_ADC
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_ADC_AnalogSource
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_Curve
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_Rally
    ${LIB_DIR}/ardu/ardupilot/libraries/AC_PID
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_Notify
    ${LIB_DIR}/ardu/ardupilot/libraries/RC_Channel
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_Motors
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_Buffer
    ${LIB_DIR}/ardu/ardupilot/libraries/AC_AttitudeControl
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_InertialNav
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_OpticalFlow
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_Relay
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_ServoRelayEvents
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_Camera
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_Mount
    ${LIB_DIR}/ardu/ardupilot/libraries/AC_WPNav
    ${LIB_DIR}/ardu/ardupilot/libraries/AC_Fence
    ${LIB_DIR}/ardu/ardupilot/libraries/SITL
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_Scheduler
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_RCMapper
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_BattMonitor
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_BoardConfig
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_SerialManager
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_Frsky_Telem
    ${LIB_DIR}/ardu/ardupilot/libraries/AC_Sprayer
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_EPM
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_Parachute
    ${LIB_DIR}/ardu/ardupilot/libraries/AP_Terrain
    ${LIB_DIR}/ardu/ardupilot/APMrover2
    ${LIB_DIR}/ardu/ardupilot/ArduCopter
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/../../libraries/sdk
)
set(EXTRA_LIBS
    sdk
)

ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ardu.helper.c
    COMMAND ${NODEJS} ${LIB_DIR}/sdk/ModuleGenerator.js ${CMAKE_CURRENT_SOURCE_DIR}/ardu.def.json ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ardu.def.json
)

add_library(ardu MODULE ${INC} ${SRC} ardu.helper.c)

target_link_libraries(ardu ${EXTRA_LIBS})

IF(${DO_POST_BUILD})
    add_custom_command(
        TARGET ardu
        POST_BUILD
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} libardu.so ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/ardu/libardu.so
        COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} ${MOD_DIR}/ardu/ardu.def.json ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/modules/ardu/ardu.def.json
    )
ENDIF()

install(TARGETS ardu DESTINATION modules/ardu)
install(FILES ardu.def.json DESTINATION modules/ardu)
