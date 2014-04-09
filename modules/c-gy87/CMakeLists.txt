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

set(EXTRA_LIBS
	c-sdk
)

add_library(c-gy87 MODULE ${INC} ${SRC})

target_link_libraries(c-gy87 ${EXTRA_LIBS})

add_custom_command(
	TARGET c-gy87
	POST_BUILD
	
	#COMMAND
	
        #COMMAND scp libc-gy87.so root@osen73.dyndns-home.com:/usr/local/linuxdrone/modules/c-gy87/libc-gy87.so

	#Windows
        COMMAND D:/Projects/LinuxDrone/tools/pscp.exe libc-gy87.so root@osen73.dyndns-home.com:/usr/local/linuxdrone/modules/c-gy87/libc-gy87.so
        #COMMAND D:/Projects/LinuxDrone/tools/pscp.exe -P 1234 libc-gy87.so root@voha-bbb.linuxdrone.org:/usr/local/linuxdrone/modules/c-gy87/libc-gy87.so
)

install(TARGETS c-gy87 DESTINATION modules/c-gy87)
install(FILES c-gy87.def.json DESTINATION modules/c-gy87)
