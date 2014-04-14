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

include_directories(${RFS_DIR}/usr/local/include/libbson-1.0)
include_directories(${RFS_DIR}/usr/local/include/libmongoc-1.0)
include_directories(${LIB_DIR}/c-sdk/include)

set(EXTRA_LIBS
    c-sdk
    ${XENOMAI_LIBRARY_NATIVE} ${XENOMAI_LIBRARY_XENOMAI}
    ${RFS_DIR}/usr/local/lib/libbson-1.0.so
    ${RFS_DIR}/usr/local/lib/libmongoc-1.0.so
    rt
    dl
    pthread
)

add_executable(c-host ${INC} ${SRC})
target_link_libraries(c-host ${EXTRA_LIBS})

add_custom_command(
    TARGET c-host
    POST_BUILD
    #COMMAND scp c-host root@osen73.dyndns-home.com:/root/c-host

    #Windows
    COMMAND ${TLS_DIR}/pscp.exe c-host root@osen73.dyndns-home.com:/root/c-host
    #COMMAND ${TLS_DIR}/pscp.exe -P 1234 c-host root@voha-bbb.linuxdrone.org:/root/c-host
    #COMMAND ${TLS_DIR}/pscp.exe -P 1234 D:/Projects/LinuxDrone/host-programs/c-host/testConfig.json root@voha-bbb.linuxdrone.org:/root/testConfig.json
    COMMAND ${TLS_DIR}/pscp.exe D:/Projects/LinuxDrone/host-programs/c-host/testConfig.json root@osen73.dyndns-home.com:/root/testConfig.json

    #COMMAND

    #Unix
    #COMMAND scp /home/vrubel/Projects/LinuxDrone/host-programs/c-host/testConfig.json root@osen73.dyndns-home.com:/root/testConfig.json
    #COMMAND scp /Users/vrubel/LinuxDrone/host-programs/c-host/testConfig.json root@osen73.dyndns-home.com:/root/testConfig.json
    #COMMAND ${TLS_DIR}/pscp.exe D:/Projects/LinuxDrone/host-programs/c-host/testConfig.json root@osen73.dyndns-home.com:/root/testConfig.json
)


install(TARGETS c-host DESTINATION bin)
