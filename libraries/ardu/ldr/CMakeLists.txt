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
    ../ardupilot/libraries/AP_HAL
    ../ardupilot/libraries/AP_Progmem
)



file(GLOB AP_HAL_LDR_INC "AP_HAL_LDR/*.h")
file(GLOB AP_HAL_LDR_SRC "AP_HAL_LDR/*.cpp")
add_library(AP_HAL_LDR ${AP_HAL_LDR_INC} ${AP_HAL_LDR_SRC})




