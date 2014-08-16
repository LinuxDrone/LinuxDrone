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

#  Find the MongoC includes and library
#  This module defines
#  MongoC_FOUND - system has the MongoC library
#  MongoC_INCLUDE_DIR, where to find libmongoc-1.0/mongoc.h
#  MongoC_LIBRARIES, the libraries needed to use MongoC.
#  MongoC_VERSION - This is set to $major.$minor.$revision$path (eg. 0.4.1)

if (UNIX)
  find_package(PkgConfig QUIET)
  pkg_check_modules(_MongoC QUIET libmongoc-1.0)
endif ()

find_path(MongoC_INCLUDE_DIR
  NAMES
    libmongoc-1.0/mongoc.h
  HINTS
    ${MongoC_ROOT_DIR}
    ${_MongoC_INCLUDEDIR}
  PATH_SUFFIXES
    /usr/includes/
    /usr/local/includes/
)

set(MongoC_INCLUDE_DIR "${MongoC_INCLUDE_DIR}/libmongoc-1.0")

if(WIN32 AND NOT CYGWIN)
  if(MSVC)
    find_library(MongoC
      NAMES
        "mongoc-1.0"
      HINTS
        ${MongoC_ROOT_DIR}
      PATH_SUFFIXES
        /usr/lib
        /usr/local/lib
    )

    mark_as_advanced(MongoC)
    set(MongoC_LIBRARIES ${MongoC} ws2_32)
  else()
      # bother supporting this?
  endif()
else()

  find_library(MongoC_LIBRARY
    NAMES
      mongoc-1.0
    HINTS
      ${_MongoC_LIBDIR}
    PATH_SUFFIXES
      lib
  )

  mark_as_advanced(MongoC_LIBRARY)

  set(MongoC_LIBRARIES ${MongoC_LIBRARY})

endif()

if (MongoC_INCLUDE_DIR)
  if (_MongoC_VERSION)
     set(MongoC_VERSION "${_MongoC_VERSION}")
  elseif(MongoC_INCLUDE_DIR AND EXISTS "${MongoC_INCLUDE_DIR}/mongoc-version.h")
     file(STRINGS "${MongoC_INCLUDE_DIR}/mongoc-version.h" MongoC_version_str
        REGEX "^#define[\t ]+MongoC_VERSION[\t ]+\([0-9.]+\)[\t ]+$")

     string(REGEX REPLACE "^.*MongoC_VERSION[\t ]+\([0-9.]+\)[\t ]+$"
        "\\1" MongoC_VERSION "${MongoC_version_str}")
  endif ()
endif ()

include(FindPackageHandleStandardArgs)

if (MongoC_VERSION)
   find_package_handle_standard_args(MongoC
    REQUIRED_VARS
      MongoC_LIBRARIES
      MongoC_INCLUDE_DIR
    VERSION_VAR
      MongoC_VERSION
    FAIL_MESSAGE
      "Could NOT find MongoC version"
  )
else ()
   find_package_handle_standard_args(MongoC "Could NOT find MongoC uuuurh"
      MongoC_LIBRARIES
      MongoC_INCLUDE_DIR
  )
endif ()

mark_as_advanced(MongoC_INCLUDE_DIR MongoC_LIBRARIES)
