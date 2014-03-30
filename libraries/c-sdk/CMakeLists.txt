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

add_library(c-sdk ${INC} ${SRC})

#set_property(TARGET c-sdk PROPERTY
#                IMPORTED_LOCATION /Users/vrubel/LinuxDrone/tools/rootfs/usr/local/xenomai/lib/libnative.a)

set(EXTRA_LIBS
	${XENOMAI_LIBRARY_NATIVE} ${XENOMAI_LIBRARY_XENOMAI}
	bson_shared
)

target_link_libraries(c-sdk ${EXTRA_LIBS})

install(TARGETS c-sdk DESTINATION lib)
