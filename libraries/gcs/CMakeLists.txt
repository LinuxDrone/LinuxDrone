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
set(SRC
	src/uavobject/cuavobject.cpp
	src/uavobject/cuavobjectfield.cpp
)

include_directories(${LIB_DIR}/core/include)

add_library(gcs ${INC} ${SRC})

install(TARGETS gcs DESTINATION lib)
