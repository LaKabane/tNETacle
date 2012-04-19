# Find tapcfg
#
# Once done, this will define:
#
#  TAPCFG_FOUND - system has Tapcfg compat
#  TAPCFG_INCLUDE_DIRS - the Tapcfg compat include directories
#  TAPCFG_LIBRARIES - link these to use Tapcfg compat
#

include(LibFindMacros)

if (TAPCFG_INCLUDE_DIR AND TAPCFG_LIBRARY)
  # Already in cache, be silent
  set(TAPCFG_FIND_QUIETLY TRUE)
endif ()

libfind_pkg_check_modules(TAPCFG_PKGCONF libtapcfg)

find_path(TAPCFG_INCLUDE_DIR tapcfg.h
  PATH ${TAPCFG_PKGCONF_INCLUDE_DIRS}
)

find_library(TAPCFG_LIBRARY
  NAMES tapcfg
  PATHS ${TAPCFG_PKGCONF_LIBRARY_DIRS}
)

set(TAPCFG_PROCESS_INCLUDES TAPCFG_INCLUDE_DIR)
set(TAPCFG_PROCESS_LIBS TAPCFG_LIBRARY)
libfind_process(TAPCFG)
