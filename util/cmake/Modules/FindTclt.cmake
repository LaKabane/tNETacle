# Find tclt
#
# Once done, this will define:
#
#  TCLT_FOUND - system has Tclt
#  TCLT_INCLUDE_DIRS - the Tclt include directories
#  TCLT_LIBRARIES - link these to use Tclt library
#

include(LibFindMacros)

if (TCLT_FIND_REQUIRED)
	set(TCLT_FIND_REQUIRED)
endif()

libfind_pkg_check_modules(TCLT_PKGCONF libtclt)

find_path(TCLT_INCLUDE_DIR tclt_json.h
  PATH ${TCLT_PKGCONF_INCLUDE_DIRS}
)

libfind_library(TCLT tclt)

set(TCLT_PROCESS_INCLUDES TCLT_INCLUDE_DIR)
set(TCLT_PROCESS_LIBS TCLT_LIBRARY)
libfind_process(TCLT)
