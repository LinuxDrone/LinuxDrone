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
file(GLOB_RECURSE SRC "src/*.cpp")

include_directories(${LIB_DIR}/core/include)
include_directories(${LIB_DIR}/sdk/include)

#set(BUILD_SHARED_LIBS TRUE)

add_library(dotnet-sdk ${INC} ${SRC})

set(EXTRA_LIBS
	#sdk
	#core
	#${MongoDB_LIBRARIES}
	#${Boost_LIBRARIES}
	${XENOMAI_LIBRARY_NATIVE} ${XENOMAI_LIBRARY_XENOMAI}
	#${CRYPTO_LIBRARIES}
	#${OPENSSL_LIBRARIES}
	#${ZLIB_LIBRARIES}
	#${CMAKE_THREAD_LIBS_INIT}
	#dl
	#rt
)

target_link_libraries(dotnet-sdk ${EXTRA_LIBS})

install(TARGETS dotnet-sdk DESTINATION lib)
