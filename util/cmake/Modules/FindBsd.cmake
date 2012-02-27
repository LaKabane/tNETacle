# Find Bsd compat lib
#
# Once done, this will define:
#
#  Bsd_FOUND - system has Bsd compat
#  Bsd_INCLUDE_DIRS - the Bsd compat include directories
#  Bsd_LIBRARIES - link these to use Bsd compat
#

include(LibFindMacros)

if (BSD_INCLUDE_DIR AND BSD_LIBRARY)
  # Already in cache, be silent
  set(BSD_FIND_QUIETLY TRUE)
endif ()

libfind_pkg_check_modules(BSD_PKGCONF libbsd)

find_path(BSD_INCLUDE_DIR bsd.h
  PATH ${BSD_PKGCONF_INCLUDE_DIRS}
  PATH_SUFFIXES bsd
)

find_library(BSD_LIBRARY
  NAMES bsd
  PATHS ${BSD_PKGCONF_LIBRARY_DIRS}
)

set(BSD_PROCESS_INCLUDES BSD_INCLUDE_DIR)
set(BSD_PROCESS_LIBS BSD_LIBRARY)
libfind_process(BSD)
