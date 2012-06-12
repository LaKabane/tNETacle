# Find yajl
# http://lloyd.github.com/yajl
#
# Once done, this will define:
#
#  YAJL_FOUND - system has Tapcfg compat
#  YAJL_INCLUDE_DIRS - the Tapcfg compat include directories
#  YAJL_LIBRARIES - link these to use Tapcfg compat
#

include(LibFindMacros)

if (YAJL_INCLUDE_DIR AND YAJL_LIBRARY)
  # Already in cache, be silent
  set(YAJL_FIND_QUIETLY TRUE)
endif ()

libfind_pkg_check_modules(YAJL_PKGCONF libyajl)

find_path(YAJL_INCLUDE_DIR
  PATH ${YAJL_PKGCONF_INCLUDE_DIRS}
)

find_library(YAJL_LIBRARY
  NAMES yajl
  PATHS ${YAJL_PKGCONF_LIBRARY_DIRS}
)

set(YAJL_PROCESS_INCLUDES YAJL_INCLUDE_DIR)
set(YAJL_PROCESS_LIBS YAJL_LIBRARY)
libfind_process(YAJL)
