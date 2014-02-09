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

file(GLOB_RECURSE SRC_BASE    "${MOD_DIR}/base/*.cpp")
file(GLOB_RECURSE SRC_MPU6050 "${MOD_DIR}/mpu6050/*.cpp")
file(GLOB_RECURSE SRC_SENSORS "${MOD_DIR}/sensors/*.cpp")
file(GLOB_RECURSE SRC_SYSTEM  "${SYS_DIR}/*.cpp")

set(SRC
	beaglebone.cpp 
	${SRC_BASE}
	${SRC_MPU6050}
	${SRC_SENSORS}
	${SRC_SYSTEM}
)

include_directories(${LIB_DIR}/core/include)

set(EXTRA_LIBS
	core
	${MongoDB_LIBRARIES}
	${Boost_LIBRARIES}
	${XENOMAI_LIBRARY_NATIVE} ${XENOMAI_LIBRARY_XENOMAI}
	${CRYPTO_LIBRARIES}
	${OPENSSL_LIBRARIES}
	${ZLIB_LIBRARIES}
)

add_executable(linuxdrone ${SRC})
target_link_libraries(linuxdrone ${EXTRA_LIBS})

install(TARGETS linuxdrone DESTINATION bin)
