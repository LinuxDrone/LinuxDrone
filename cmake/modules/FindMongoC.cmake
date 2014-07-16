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

# - Find MongoC
#  Find the MongoC includes and library
#  This module defines
#  MongoC_INCLUDE_DIR, where to find mongo/client/dbclient.h
#  MongoC_LIBRARIES, the libraries needed to use MongoC.
#  MongoC_Found, If false, do not try to use MongoC.

if(MongoC_INCLUDE_DIR AND MongoC_LIBRARIES)
   set(MongoC_Found TRUE)

else(MongoC_INCLUDE_DIR AND MongoC_LIBRARIES)

  find_path(
      MongoC_INCLUDE_DIR
      NAMES libmongoc-1.0/mongoc.h
      /usr/include/
      /usr/local/include/
      )

  set(MongoC_INCLUDE_DIR "${MongoC_INCLUDE_DIR}/libmongoc-1.0")

  find_library(
      MongoC_LIBRARIES
      NAMES mongoc-1.0
      #PATHS
      /usr/lib
      /usr/local/lib
      )

  if(MongoC_INCLUDE_DIR AND MongoC_LIBRARIES)
    set(MongoC_Found TRUE)
    message(STATUS "Found MongoC: ${MongoC_INCLUDE_DIR}, ${MongoC_LIBRARIES}")
  else(MongoC_INCLUDE_DIR AND MongoC_LIBRARIES)
    set(MongoC_FOUND FALSE)
    if (MongoC_FIND_REQUIRED)
		message(FATAL_ERROR "MongoC not found.")
	else (MongoC_FIND_REQUIRED)
		message(STATUS "MongoC not found.")
	endif (MongoC_FIND_REQUIRED)
  endif(MongoC_INCLUDE_DIR AND MongoC_LIBRARIES)

  mark_as_advanced(MongoC_INCLUDE_DIR MongoC_LIBRARIES)

endif(MongoC_INCLUDE_DIR AND MongoC_LIBRARIES)
