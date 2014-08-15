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

# - Find WebSockets
#  Find the WebSockets includes and library
#  This module defines
#  WebSockets_INCLUDE_DIR, where to find mongo/client/dbclient.h
#  WebSockets_LIBRARIES, the libraries needed to use WebSockets.
#  WebSockets_Found, If false, do not try to use WebSockets.

if(WebSockets_INCLUDE_DIR AND WebSockets_LIBRARIES)
   set(WebSockets_Found TRUE)

else(WebSockets_INCLUDE_DIR AND WebSockets_LIBRARIES)

  find_path(
      WebSockets_INCLUDE_DIR
      NAMES libwebsockets.h
      /usr/include/
      /usr/local/include/
      )

  find_library(
      WebSockets_LIBRARIES
      NAMES websockets
      /usr/lib
      /usr/local/lib
      )

  if(WebSockets_INCLUDE_DIR AND WebSockets_LIBRARIES)
    set(WebSockets_Found TRUE)
    message(STATUS "Found WebSockets: ${WebSockets_INCLUDE_DIR}, ${WebSockets_LIBRARIES}")
  else(WebSockets_INCLUDE_DIR AND WebSockets_LIBRARIES)
    set(WebSockets_FOUND FALSE)
    if (WebSockets_FIND_REQUIRED)
		message(FATAL_ERROR "WebSockets not found.")
	else (WebSockets_FIND_REQUIRED)
		message(STATUS "WebSockets not found.")
	endif (WebSockets_FIND_REQUIRED)
  endif(WebSockets_INCLUDE_DIR AND WebSockets_LIBRARIES)

  mark_as_advanced(WebSockets_INCLUDE_DIR WebSockets_LIBRARIES)

endif(WebSockets_INCLUDE_DIR AND WebSockets_LIBRARIES)
