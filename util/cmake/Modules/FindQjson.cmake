# Find libqjson
#
# Once done, this will define:
#
#  QJSON_FOUND - system has libqjson compat
#  QJSON_INCLUDE_DIRS - the libqjson compat include directories
#  QJSON_LIBRARIES - link these to use libqjson compat
#

include(LibFindMacros)

if (QJSON_FIND_REQUIRED)
	set(QJSON_FIND_REQUIRED)
endif()

libfind_pkg_check_modules(QJSON_PKGCONF QJson)

find_path(QJSON_INCLUDE_DIR qjson/parser.h
  PATH ${QJSON_PKGCONF_INCLUDE_DIRS}
)

find_library(QJSON_LIBRARY qjson)

set(QJSON_PROCESS_INCLUDES QJSON_INCLUDE_DIR)
set(QJSON_PROCESS_LIBS QJSON_LIBRARY)
libfind_process(QJSON)
