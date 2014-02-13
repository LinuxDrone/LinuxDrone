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
file(GLOB_RECURSE SRC "src/*.cpp")

include_directories(${LIB_DIR}/core/include)
include_directories(${LIB_DIR}/sdk/include)

set(EXTRA_LIBS
	sdk
	core
)

add_library(ms5611 MODULE ${INC} ${SRC})

install(TARGETS ms5611 DESTINATION modules/ms5611)
install(FILES ms5611.def.json DESTINATION modules/ms5611)
