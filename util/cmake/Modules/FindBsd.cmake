# Find Bsd compat lib
#
# Once done, this will define:
#
#  Bsd_FOUND - system has Bsd compat
#  Bsd_INCLUDE_DIRS - the Bsd compat include directories
#  Bsd_LIBRARIES - link these to use Bsd compat
#

if (BSD_INCLUDE_DIR AND BSD_LIBRARY)
  # Already in cache, be silent
  set(BSD_FIND_QUIETLY TRUE)
endif (BSD_INCLUDE_DIR AND BSD_LIBRARY)

find_path(BSD_INCLUDE_DIR bsd.h
  PATHS /usr/include /usr/local/include
  PATH_SUFFIXES bsd
)

find_library(BSD_LIBRARY
  NAMES bsd
  PATHS /usr/lib /usr/local/lib
)

set(BSD_LIBRARIES ${BSD_LIBRARY} )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BSD
  DEFAULT_MSG
  BSD_INCLUDE_DIR
  BSD_LIBRARIES
)

mark_as_advanced(BSD_INCLUDE_DIR BSD_LIBRARY)
