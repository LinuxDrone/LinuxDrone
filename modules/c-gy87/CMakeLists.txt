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

include_directories(${LIB_DIR}/c-sdk/include)
include_directories(../../tools/rootfs/beaglebone/usr/local/include/libbson-1.0)

#set(EXTRA_LIBS
#	sdk

#	core
#)

add_library(c-gy87 MODULE ${INC} ${SRC})

install(TARGETS c-gy87 DESTINATION modules/c-gy87)
install(FILES c-gy87.def.json DESTINATION modules/c-gy87)
