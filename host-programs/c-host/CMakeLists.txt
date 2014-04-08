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

include_directories(../../tools/rootfs/beaglebone/usr/local/include/libbson-1.0)
include_directories(../../modules/c-gy87/include)


set(EXTRA_LIBS
        #D:/Projects/LinuxDrone-build-work/libraries/c-sdk/libc-sdk.so
        #D:/Projects/LinuxDrone-build-work/modules/c-gy87/libc-gy87.so
        /Users/vrubel/LinuxDrone/build.Debug/libraries/c-sdk/libc-sdk.so
        /Users/vrubel/LinuxDrone/build.Debug/modules/c-gy87/libc-gy87.so
	#/home/vrubel/Projects/LinuxDrone/build.Debug/libraries/c-sdk/libc-sdk.so
	#/home/vrubel/Projects/LinuxDrone/build.Debug/modules/c-gy87/libc-gy87.so
	#c-gy87
	${XENOMAI_LIBRARY_NATIVE} ${XENOMAI_LIBRARY_XENOMAI}
        bson_shared
        #D:/Projects/LinuxDrone/tools/rootfs/beaglebone/usr/local/lib/libbson-1.0.so
	rt
)

add_executable(c-host ${INC} ${SRC})
target_link_libraries(c-host ${EXTRA_LIBS})

add_custom_command(
	TARGET c-host
	POST_BUILD
        COMMAND scp c-host root@osen73.dyndns-home.com:/root/c-host

    #Windows
    #COMMAND D:/Projects/LinuxDrone/tools/pscp.exe c-host root@osen73.dyndns-home.com:/root/c-host
    #COMMAND D:/Projects/LinuxDrone/tools/pscp.exe -P 1234 c-host root@voha-bbb.linuxdrone.org:/root/c-host
    #COMMAND D:/Projects/LinuxDrone/tools/pscp.exe -P 1234 D:/Projects/LinuxDrone/host-programs/c-host/testConfig.json root@voha-bbb.linuxdrone.org:/root/testConfig.json

#COMMAND

	#COMMAND scp /home/vrubel/Projects/LinuxDrone/host-programs/c-host/testConfig.json root@osen73.dyndns-home.com:/root/testConfig.json
    COMMAND scp /Users/vrubel/LinuxDrone/host-programs/c-host/testConfig.json root@osen73.dyndns-home.com:/root/testConfig.json
)


install(TARGETS c-host DESTINATION bin)
