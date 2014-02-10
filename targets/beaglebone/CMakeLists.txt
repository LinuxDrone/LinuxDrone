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

set(SRC
	beaglebone.cpp 
)

include_directories(${LIB_DIR}/core/include)
include_directories(${LIB_DIR}/sdk/include)

set(EXTRA_LIBS
	sdk
	core
	${MongoDB_LIBRARIES}
	${Boost_LIBRARIES}
	${XENOMAI_LIBRARY_NATIVE} ${XENOMAI_LIBRARY_XENOMAI}
	${CRYPTO_LIBRARIES}
	${OPENSSL_LIBRARIES}
	${ZLIB_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
)

add_executable(linuxdrone ${SRC})
target_link_libraries(linuxdrone ${EXTRA_LIBS})

install(TARGETS linuxdrone DESTINATION bin)
