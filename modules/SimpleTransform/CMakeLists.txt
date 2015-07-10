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

file(GLOB_RECURSE SRC "src/*.cpp")

include_directories(${LIB_DIR}/core/include)
include_directories(${LIB_DIR}/sdk/include)

set(EXTRA_LIBS
	sdk
	core
    pthread
    rt
)

add_executable(simpletransform ${INC} ${SRC})

install(TARGETS simpletransform DESTINATION bin)
