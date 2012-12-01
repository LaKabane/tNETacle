# Find tuntap
#
# Once done, this will define:
#
#  TUNTAP_FOUND - system has Tapcfg compat
#  TUNTAP_INCLUDE_DIRS - the Tapcfg compat include directories
#  TUNTAP_LIBRARIES - link these to use Tapcfg compat
#

include(LibFindMacros)

if (Tuntap_FIND_REQUIRED)
	set(TUNTAP_FIND_REQUIRED)
endif()

libfind_pkg_check_modules(TUNTAP_PKGCONF libtuntap)

find_path(TUNTAP_INCLUDE_DIR tuntap.h
        PATHS ${TUNTAP_PKGCONF_INCLUDE_DIRS}
              ${CMAKE_SOURCE_DIR}/sub/libtuntap/
              ${CMAKE_BINARY_DIR}/libtuntap/
              ${CMAKE_BINARY_DIR}/sub/libtuntap/
)

# Best effort if we don't find it..
find_library(TUNTAP_LIBRARY
        NAMES tuntap
        PATHS ${CMAKE_SOURCE_DIR}/sub/libtuntap/lib/
              ${CMAKE_BINARY_DIR}/libtuntap/lib
              ${CMAKE_BINARY_DIR}/sub/libtuntap/lib
)

set(TUNTAP_PROCESS_INCLUDES TUNTAP_INCLUDE_DIR)
set(TUNTAP_PROCESS_LIBS TUNTAP_LIBRARY)
libfind_process(TUNTAP)
