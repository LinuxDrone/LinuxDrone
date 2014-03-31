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

include_directories(../../../../tools/rootfs/beaglebone/usr/local/include/libbson-1.0)

add_executable(app1 ${INC} ${SRC})

set(EXTRA_LIBS
	/Users/vrubel/LinuxDrone/build.Debug/libraries/c-sdk/libc-sdk.so
	${XENOMAI_LIBRARY_NATIVE} ${XENOMAI_LIBRARY_XENOMAI}
	bson_shared
	rt
)

target_link_libraries(app1 ${EXTRA_LIBS})

ADD_CUSTOM_TARGET(app1copy
        COMMAND scp app1 root@osen73.dyndns-home.com:/root/app1
)

install(TARGETS app1 DESTINATION lib)
