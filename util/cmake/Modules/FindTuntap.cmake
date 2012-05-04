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
  PATH ${TUNTAP_PKGCONF_INCLUDE_DIRS}
)

libfind_library(TUNTAP tuntap)

set(TUNTAP_PROCESS_INCLUDES TUNTAP_INCLUDE_DIR)
set(TUNTAP_PROCESS_LIBS TUNTAP_LIBRARY)
libfind_process(TUNTAP)
