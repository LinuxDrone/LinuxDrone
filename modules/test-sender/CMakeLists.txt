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

add_library(test-sender MODULE ${INC} ${SRC})

target_link_libraries(test-sender ${EXTRA_LIBS})

add_custom_command(
	TARGET test-sender
	POST_BUILD
	
	#COMMAND
	
        #COMMAND scp libtest-sender.so root@osen73.dyndns-home.com:/usr/local/linuxdrone/modules/test-sender/libtest-sender.so

	#Windows
        COMMAND D:/Projects/LinuxDrone/tools/pscp.exe libtest-sender.so root@osen73.dyndns-home.com:/usr/local/linuxdrone/modules/test-sender/libtest-sender.so
        #COMMAND D:/Projects/LinuxDrone/tools/pscp.exe -P 1234 libtest-sender.so root@voha-bbb.linuxdrone.org:/usr/local/linuxdrone/modules/test-sender/libtest-sender.so
)

install(TARGETS test-sender DESTINATION modules/test-sender)
install(FILES test-sender.def.json DESTINATION modules/test-sender)
