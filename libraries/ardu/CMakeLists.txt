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

set(CONFIG_HAL_BOARD HAL_BOARD_LINUX)

add_definitions(-DCONFIG_HAL_BOARD=HAL_BOARD_LINUX)
add_definitions(-DCONFIG_HAL_BOARD_SUBTYPE=HAL_BOARD_SUBTYPE_LINUX_ERLE)

# -DF_CPU
#-Wunused-parameter
#-Wunused-variable
add_definitions(-DSKETCHNAME="ArduCopter")

#file(GLOB_RECURSE INC "include/*.h")
#file(GLOB_RECURSE SRC "src/*.c")



#include_directories(${RFS_DIR}/usr/local/include/libbson-1.0 ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include)

include_directories(
    ardupilot/libraries/AP_Common
    ardupilot/libraries/AP_HAL
    ardupilot/libraries/AP_Progmem
    ardupilot/libraries/AP_Rally
    ardupilot/libraries/AP_ADC
    ardupilot/libraries/AP_ADC_AnalogSource
    ardupilot/libraries/AP_Terrain
    ardupilot/libraries/AP_Mission
    ardupilot/libraries/GCS_MAVLink
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
)


#set(EXTRA_LIBS
#    ${XENOMAI_LIBRARY_NATIVE} ${XENOMAI_LIBRARY_XENOMAI}
#    ${RFS_DIR}/usr/local/lib/libbson-1.0.so
#    m
#)

#ADD_CUSTOM_COMMAND(
 #   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/common-params.c
  #  COMMAND ${NODEJS} ${LIB_DIR}/sdk/CommonParamsGenerator.js ${CMAKE_CURRENT_SOURCE_DIR}/../../webapps/configurator/public/ModulesCommonParams.def.js ${CMAKE_CURRENT_BINARY_DIR}
   # DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../../webapps/configurator/public/ModulesCommonParams.def.js
#)


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



file(GLOB_RECURSE AP_AHRS_INC "ardupilot/libraries/AP_AHRS/*.h")
file(GLOB_RECURSE AP_AHRS_SRC "ardupilot/libraries/AP_AHRS/*.cpp")
add_library(AP_AHRS ${AP_AHRS_INC} ${AP_AHRS_SRC})

file(GLOB_RECURSE AP_Baro_INC "ardupilot/libraries/AP_Baro/*.h")
file(GLOB_RECURSE AP_Baro_SRC "ardupilot/libraries/AP_Baro/*.cpp")
add_library(AP_Baro ${AP_Baro_INC} ${AP_Baro_SRC})



#target_link_libraries(sdk ${EXTRA_LIBS})

#IF(${DO_POST_BUILD})
 #   add_custom_command(
  #      TARGET sdk
   #     POST_BUILD
    #    COMMAND ${SCP} -P ${SSH_PORT_TARGET_SYSTEM} libsdk.so ${URL_TARGET_SYSTEM}:/usr/local/linuxdrone/lib/libsdk.so
    #)
#ENDIF()

#install(TARGETS sdk DESTINATION lib)
