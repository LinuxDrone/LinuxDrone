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

include_directories(
    ardupilot/libraries/AP_Common
    ardupilot/libraries/AP_HAL
    ardupilot/libraries/AP_HAL_AVR
    ardupilot/libraries/AP_HAL_AVR_SITL
    ardupilot/libraries/AP_HAL_PX4
    ardupilot/libraries/AP_HAL_VRBRAIN
    ardupilot/libraries/AP_HAL_FLYMAPLE
    ardupilot/libraries/AP_Progmem
    ardupilot/libraries/AP_Rally
    ardupilot/libraries/AP_ADC
    ardupilot/libraries/AP_ADC_AnalogSource
    ardupilot/libraries/AP_Terrain
    ardupilot/libraries/AP_Mission
    ardupilot/libraries/GCS_MAVLink
    ardupilot/libraries/AP_Menu
    ardupilot/libraries/AP_Param
    ardupilot/libraries/AP_Math
    ardupilot/libraries/SITL
    ardupilot/libraries/DataFlash
    ardupilot/libraries/AP_GPS
    ardupilot/libraries/AP_Vehicle
    ardupilot/libraries/AP_HAL_Empty
    ardupilot/libraries/AP_HAL_Linux
    ardupilot/libraries/AP_InertialSensor
    ardupilot/libraries/Filter
    ardupilot/libraries/AP_Baro
    ardupilot/libraries/AP_AHRS
    ardupilot/libraries/AP_Compass
    ardupilot/libraries/AP_Declination
    ardupilot/libraries/AP_Airspeed
    ardupilot/libraries/AP_NavEKF
    ardupilot/libraries/AP_Notify
    ardupilot/libraries/StorageManager
    ardupilot/libraries/AP_Curve
    ardupilot/libraries/AC_PID
    ardupilot/libraries/AP_Motors
    ardupilot/libraries/RC_Channel
    ardupilot/libraries/AP_Buffer
    ardupilot/libraries/AP_InertialNav
    ardupilot/libraries/AC_AttitudeControl
    ardupilot/libraries/AP_RangeFinder
    ardupilot/libraries/AP_OpticalFlow
    ardupilot/libraries/AP_Relay
    ardupilot/libraries/AP_ServoRelayEvents
    ardupilot/libraries/AP_Camera
    ardupilot/libraries/AP_Mount
    ardupilot/libraries/AC_WPNav
    ardupilot/libraries/AC_Fence
    ardupilot/libraries/AP_Scheduler
    ardupilot/libraries/AP_RCMapper
    ardupilot/libraries/AP_BattMonitor
    ardupilot/libraries/AP_BoardConfig
    ardupilot/libraries/AP_Frsky_Telem
    ardupilot/libraries/AC_Sprayer
    ardupilot/libraries/AP_EPM
    ardupilot/libraries/AP_Parachute
)


file(GLOB_RECURSE AP_Common_INC "ardupilot/libraries/AP_Common/*.h")
file(GLOB_RECURSE AP_Common_SRC "ardupilot/libraries/AP_Common/*.cpp")
add_library(AP_Common ${AP_Common_INC} ${AP_Common_SRC})

file(GLOB_RECURSE AP_HAL_INC "ardupilot/libraries/AP_HAL/*.h")
file(GLOB_RECURSE AP_HAL_SRC "ardupilot/libraries/AP_HAL/*.cpp")
add_library(AP_HAL ${AP_HAL_INC} ${AP_HAL_SRC})

file(GLOB_RECURSE AP_Progmem_INC "ardupilot/libraries/AP_Progmem/*.h")
file(GLOB_RECURSE AP_Progmem_SRC "ardupilot/libraries/AP_Progmem/*.cpp")
add_library(AP_Progmem ${AP_Progmem_INC} ${AP_Progmem_SRC})

file(GLOB_RECURSE AP_Rally_INC "ardupilot/libraries/AP_Rally/*.h")
file(GLOB_RECURSE AP_Rally_SRC "ardupilot/libraries/AP_Rally/*.cpp")
add_library(AP_Rally ${AP_Rally_INC} ${AP_Rally_SRC})

file(GLOB_RECURSE AP_ADC_INC "ardupilot/libraries/AP_ADC/*.h")
file(GLOB_RECURSE AP_ADC_SRC "ardupilot/libraries/AP_ADC/*.cpp")
add_library(AP_ADC ${AP_ADC_INC} ${AP_ADC_SRC})

file(GLOB_RECURSE AP_ADC_AnalogSource_INC "ardupilot/libraries/AP_ADC_AnalogSource/*.h")
file(GLOB_RECURSE AP_ADC_AnalogSource_SRC "ardupilot/libraries/AP_ADC_AnalogSource/*.cpp")
add_library(AP_ADC_AnalogSource ${AP_ADC_AnalogSource_INC} ${AP_ADC_AnalogSource_SRC})

file(GLOB_RECURSE AP_Terrain_INC "ardupilot/libraries/AP_Terrain/*.h")
file(GLOB_RECURSE AP_Terrain_SRC "ardupilot/libraries/AP_Terrain/*.cpp")
add_library(AP_Terrain ${AP_Terrain_INC} ${AP_Terrain_SRC})

file(GLOB_RECURSE AP_Mission_INC "ardupilot/libraries/AP_Mission/*.h")
file(GLOB_RECURSE AP_Mission_SRC "ardupilot/libraries/AP_Mission/*.cpp")
add_library(AP_Mission ${AP_Mission_INC} ${AP_Mission_SRC})

file(GLOB_RECURSE GCS_MAVLink_INC "ardupilot/libraries/GCS_MAVLink/*.h")
file(GLOB_RECURSE GCS_MAVLink_SRC "ardupilot/libraries/GCS_MAVLink/*.cpp")
add_library(GCS_MAVLink ${GCS_MAVLink_INC} ${GCS_MAVLink_SRC})

file(GLOB_RECURSE AP_Menu_INC "ardupilot/libraries/AP_Menu/*.h")
file(GLOB_RECURSE AP_Menu_SRC "ardupilot/libraries/AP_Menu/*.cpp")
add_library(AP_Menu ${AP_Menu_INC} ${AP_Menu_SRC})

file(GLOB_RECURSE AP_Param_INC "ardupilot/libraries/AP_Param/*.h")
file(GLOB_RECURSE AP_Param_SRC "ardupilot/libraries/AP_Param/*.cpp")
add_library(AP_Param ${AP_Param_INC} ${AP_Param_SRC})

file(GLOB_RECURSE AP_Math_INC "ardupilot/libraries/AP_Math/*.h")
file(GLOB_RECURSE AP_Math_SRC "ardupilot/libraries/AP_Math/*.cpp")
add_library(AP_Math ${AP_Math_INC} ${AP_Math_SRC})

file(GLOB_RECURSE SITL_INC "ardupilot/libraries/SITL/*.h")
file(GLOB_RECURSE SITL_SRC "ardupilot/libraries/SITL/*.cpp")
add_library(SITL ${SITL_INC} ${SITL_SRC})

file(GLOB_RECURSE DataFlash_INC "ardupilot/libraries/DataFlash/*.h")
file(GLOB_RECURSE DataFlash_SRC "ardupilot/libraries/DataFlash/*.cpp")
add_library(DataFlash ${DataFlash_INC} ${DataFlash_SRC})

file(GLOB_RECURSE AP_GPS_INC "ardupilot/libraries/AP_GPS/*.h")
file(GLOB_RECURSE AP_GPS_SRC "ardupilot/libraries/AP_GPS/*.cpp")
add_library(AP_GPS ${AP_GPS_INC} ${AP_GPS_SRC})

file(GLOB_RECURSE AP_HAL_Empty_INC "ardupilot/libraries/AP_HAL_Empty/*.h")
file(GLOB_RECURSE AP_HAL_Empty_SRC "ardupilot/libraries/AP_HAL_Empty/*.cpp")
add_library(AP_HAL_Empty ${AP_HAL_Empty_INC} ${AP_HAL_Empty_SRC})

file(GLOB_RECURSE AP_HAL_Linux_INC "ardupilot/libraries/AP_HAL_Linux/*.h")
file(GLOB_RECURSE AP_HAL_Linux_SRC "ardupilot/libraries/AP_HAL_Linux/*.cpp")
add_library(AP_HAL_Linux ${AP_HAL_Linux_INC} ${AP_HAL_Linux_SRC})

file(GLOB_RECURSE AP_InertialSensor_INC "ardupilot/libraries/AP_InertialSensor/*.h")
file(GLOB_RECURSE AP_InertialSensor_SRC "ardupilot/libraries/AP_InertialSensor/*.cpp")
add_library(AP_InertialSensor ${AP_InertialSensor_INC} ${AP_InertialSensor_SRC})

file(GLOB_RECURSE Filter_INC "ardupilot/libraries/Filter/*.h")
file(GLOB_RECURSE Filter_SRC "ardupilot/libraries/Filter/*.cpp")
add_library(Filter ${Filter_INC} ${Filter_SRC})

file(GLOB_RECURSE AP_Baro_INC "ardupilot/libraries/AP_Baro/*.h")
file(GLOB_RECURSE AP_Baro_SRC "ardupilot/libraries/AP_Baro/*.cpp")
add_library(AP_Baro ${AP_Baro_INC} ${AP_Baro_SRC})

file(GLOB_RECURSE AP_AHRS_INC "ardupilot/libraries/AP_AHRS/*.h")
file(GLOB_RECURSE AP_AHRS_SRC "ardupilot/libraries/AP_AHRS/*.cpp")
add_library(AP_AHRS ${AP_AHRS_INC} ${AP_AHRS_SRC})

file(GLOB_RECURSE AP_Compass_INC "ardupilot/libraries/AP_Compass/*.h")
file(GLOB_RECURSE AP_Compass_SRC "ardupilot/libraries/AP_Compass/*.cpp")
add_library(AP_Compass ${AP_Compass_INC} ${AP_Compass_SRC})

file(GLOB_RECURSE AP_Declination_INC "ardupilot/libraries/AP_Declination/*.h")
file(GLOB_RECURSE AP_Declination_SRC "ardupilot/libraries/AP_Declination/*.cpp")
add_library(AP_Declination ${AP_Declination_INC} ${AP_Declination_SRC})

file(GLOB_RECURSE AP_Airspeed_INC "ardupilot/libraries/AP_Airspeed/*.h")
file(GLOB_RECURSE AP_Airspeed_SRC "ardupilot/libraries/AP_Airspeed/*.cpp")
add_library(AP_Airspeed ${AP_Airspeed_INC} ${AP_Airspeed_SRC})

file(GLOB_RECURSE AP_NavEKF_INC "ardupilot/libraries/AP_NavEKF/*.h")
file(GLOB_RECURSE AP_NavEKF_SRC "ardupilot/libraries/AP_NavEKF/*.cpp")
add_library(AP_NavEKF ${AP_NavEKF_INC} ${AP_NavEKF_SRC})

file(GLOB_RECURSE AP_Notify_INC "ardupilot/libraries/AP_Notify/*.h")
file(GLOB_RECURSE AP_Notify_SRC "ardupilot/libraries/AP_Notify/*.cpp")
add_library(AP_Notify ${AP_Notify_INC} ${AP_Notify_SRC})

file(GLOB_RECURSE StorageManager_INC "ardupilot/libraries/StorageManager/*.h")
file(GLOB_RECURSE StorageManager_SRC "ardupilot/libraries/StorageManager/*.cpp")
add_library(StorageManager ${StorageManager_INC} ${StorageManager_SRC})

file(GLOB_RECURSE AP_Curve_INC "ardupilot/libraries/AP_Curve/*.h")
file(GLOB_RECURSE AP_Curve_SRC "ardupilot/libraries/AP_Curve/*.cpp")
add_library(AP_Curve ${AP_Curve_INC} ${AP_Curve_SRC})

file(GLOB_RECURSE AC_PID_INC "ardupilot/libraries/AC_PID/*.h")
file(GLOB_RECURSE AC_PID_SRC "ardupilot/libraries/AC_PID/*.cpp")
add_library(AC_PID ${AC_PID_INC} ${AC_PID_SRC})

file(GLOB_RECURSE AP_Motors_INC "ardupilot/libraries/AP_Motors/*.h")
file(GLOB_RECURSE AP_Motors_SRC "ardupilot/libraries/AP_Motors/*.cpp")
add_library(AP_Motors ${AP_Motors_INC} ${AP_Motors_SRC})

file(GLOB_RECURSE RC_Channel_INC "ardupilot/libraries/RC_Channel/*.h")
file(GLOB_RECURSE RC_Channel_SRC "ardupilot/libraries/RC_Channel/*.cpp")
add_library(RC_Channel ${RC_Channel_INC} ${RC_Channel_SRC})

file(GLOB_RECURSE AP_InertialNav_INC "ardupilot/libraries/AP_InertialNav/*.h")
file(GLOB_RECURSE AP_InertialNav_SRC "ardupilot/libraries/AP_InertialNav/*.cpp")
add_library(AP_InertialNav ${AP_InertialNav_INC} ${AP_InertialNav_SRC})

file(GLOB_RECURSE AC_AttitudeControl_INC "ardupilot/libraries/AC_AttitudeControl/*.h")
file(GLOB_RECURSE AC_AttitudeControl_SRC "ardupilot/libraries/AC_AttitudeControl/*.cpp")
add_library(AC_AttitudeControl ${AC_AttitudeControl_INC} ${AC_AttitudeControl_SRC})

file(GLOB_RECURSE AP_RangeFinder_INC "ardupilot/libraries/AP_RangeFinder/*.h")
file(GLOB_RECURSE AP_RangeFinder_SRC "ardupilot/libraries/AP_RangeFinder/*.cpp")
add_library(AP_RangeFinder ${AP_RangeFinder_INC} ${AP_RangeFinder_SRC})

file(GLOB_RECURSE AP_OpticalFlow_INC "ardupilot/libraries/AP_OpticalFlow/*.h")
file(GLOB_RECURSE AP_OpticalFlow_SRC "ardupilot/libraries/AP_OpticalFlow/*.cpp")
add_library(AP_OpticalFlow ${AP_OpticalFlow_INC} ${AP_OpticalFlow_SRC})

file(GLOB_RECURSE AP_Relay_INC "ardupilot/libraries/AP_Relay/*.h")
file(GLOB_RECURSE AP_Relay_SRC "ardupilot/libraries/AP_Relay/*.cpp")
add_library(AP_Relay ${AP_Relay_INC} ${AP_Relay_SRC})

file(GLOB_RECURSE AP_ServoRelayEvents_INC "ardupilot/libraries/AP_ServoRelayEvents/*.h")
file(GLOB_RECURSE AP_ServoRelayEvents_SRC "ardupilot/libraries/AP_ServoRelayEvents/*.cpp")
add_library(AP_ServoRelayEvents ${AP_ServoRelayEvents_INC} ${AP_ServoRelayEvents_SRC})

file(GLOB_RECURSE AP_Camera_INC "ardupilot/libraries/AP_Camera/*.h")
file(GLOB_RECURSE AP_Camera_SRC "ardupilot/libraries/AP_Camera/*.cpp")
add_library(AP_Camera ${AP_Camera_INC} ${AP_Camera_SRC})

file(GLOB_RECURSE AP_Mount_INC "ardupilot/libraries/AP_Mount/*.h")
file(GLOB_RECURSE AP_Mount_SRC "ardupilot/libraries/AP_Mount/*.cpp")
add_library(AP_Mount ${AP_Mount_INC} ${AP_Mount_SRC})

file(GLOB_RECURSE AC_WPNav_INC "ardupilot/libraries/AC_WPNav/*.h")
file(GLOB_RECURSE AC_WPNav_SRC "ardupilot/libraries/AC_WPNav/*.cpp")
add_library(AC_WPNav ${AC_WPNav_INC} ${AC_WPNav_SRC})

file(GLOB_RECURSE AC_Fence_INC "ardupilot/libraries/AC_Fence/*.h")
file(GLOB_RECURSE AC_Fence_SRC "ardupilot/libraries/AC_Fence/*.cpp")
add_library(AC_Fence ${AC_Fence_INC} ${AC_Fence_SRC})

file(GLOB_RECURSE AP_Scheduler_INC "ardupilot/libraries/AP_Scheduler/*.h")
file(GLOB_RECURSE AP_Scheduler_SRC "ardupilot/libraries/AP_Scheduler/*.cpp")
add_library(AP_Scheduler ${AP_Scheduler_INC} ${AP_Scheduler_SRC})

file(GLOB_RECURSE AP_RCMapper_INC "ardupilot/libraries/AP_RCMapper/*.h")
file(GLOB_RECURSE AP_RCMapper_SRC "ardupilot/libraries/AP_RCMapper/*.cpp")
add_library(AP_RCMapper ${AP_RCMapper_INC} ${AP_RCMapper_SRC})

file(GLOB_RECURSE AP_BattMonitor_INC "ardupilot/libraries/AP_BattMonitor/*.h")
file(GLOB_RECURSE AP_BattMonitor_SRC "ardupilot/libraries/AP_BattMonitor/*.cpp")
add_library(AP_BattMonitor ${AP_BattMonitor_INC} ${AP_BattMonitor_SRC})

file(GLOB_RECURSE AP_BoardConfig_INC "ardupilot/libraries/AP_BoardConfig/*.h")
file(GLOB_RECURSE AP_BoardConfig_SRC "ardupilot/libraries/AP_BoardConfig/*.cpp")
add_library(AP_BoardConfig ${AP_BoardConfig_INC} ${AP_BoardConfig_SRC})

file(GLOB_RECURSE AP_Frsky_Telem_INC "ardupilot/libraries/AP_Frsky_Telem/*.h")
file(GLOB_RECURSE AP_Frsky_Telem_SRC "ardupilot/libraries/AP_Frsky_Telem/*.cpp")
add_library(AP_Frsky_Telem ${AP_Frsky_Telem_INC} ${AP_Frsky_Telem_SRC})

file(GLOB_RECURSE AC_Sprayer_INC "ardupilot/libraries/AC_Sprayer/*.h")
file(GLOB_RECURSE AC_Sprayer_SRC "ardupilot/libraries/AC_Sprayer/*.cpp")
add_library(AC_Sprayer ${AC_Sprayer_INC} ${AC_Sprayer_SRC})

file(GLOB_RECURSE AP_EPM_INC "ardupilot/libraries/AP_EPM/*.h")
file(GLOB_RECURSE AP_EPM_SRC "ardupilot/libraries/AP_EPM/*.cpp")
add_library(AP_EPM ${AP_EPM_INC} ${AP_EPM_SRC})

file(GLOB_RECURSE AP_Parachute_INC "ardupilot/libraries/AP_Parachute/*.h")
file(GLOB_RECURSE AP_Parachute_SRC "ardupilot/libraries/AP_Parachute/*.cpp")
add_library(AP_Parachute ${AP_Parachute_INC} ${AP_Parachute_SRC})


set(ArduCopter_EXTRA_LIBS
    AP_Common
    AP_HAL
    AP_Progmem
    AP_Rally
    AP_ADC
    AP_ADC_AnalogSource
    AP_Terrain
    AP_Mission
    GCS_MAVLink
    AP_Menu
    AP_Param
    AP_Math
    SITL
    DataFlash
    AP_GPS
    AP_HAL_Empty
    AP_HAL_Linux
    AP_InertialSensor
    Filter
    AP_Baro
    AP_AHRS
    AP_Compass
    AP_Declination
    AP_Airspeed
    AP_NavEKF
    AP_Notify
    StorageManager
    AP_Curve
    AC_PID
    AP_Motors
    RC_Channel
    AP_InertialNav
    AC_AttitudeControl
    AP_RangeFinder
    AP_OpticalFlow
    AP_Relay
    AP_ServoRelayEvents
    AP_Camera
    AP_Mount
    AC_WPNav
    AC_Fence
    AP_Scheduler
    AP_RCMapper
    AP_BattMonitor
    AP_BoardConfig
    AP_Frsky_Telem
    AC_Sprayer
    AP_EPM
    AP_Parachute
    #sdk
    #${XENOMAI_LIBRARY_NATIVE} ${XENOMAI_LIBRARY_XENOMAI}
    #${BSON_LIBRARIES}
    #${MongoC_LIBRARIES}
    rt
    dl
    pthread
)

file(GLOB_RECURSE ArduCopter_INC "ardupilot/ArduCopter/*.h")
file(GLOB_RECURSE ArduCopter_SRC "ardupilot/ArduCopter/ArduCopter.cpp")
add_executable(ArduCopter ${ArduCopter_INC} ${ArduCopter_SRC})
target_link_libraries(ArduCopter ${ArduCopter_EXTRA_LIBS})

