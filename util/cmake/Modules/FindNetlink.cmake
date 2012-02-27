# Find Netlink lib
#
# Once done, this will define:
#
#  Netlink_FOUND - system has Netlink compat
#  Netlink_INCLUDE_DIRS - the Netlink compat include directories
#  Netlink_LIBRARIES - link these to use Netlink compat
#

include(LibFindMacros)

if (NETLINK_INCLUDE_DIR AND NETLINK_LIBRARY)
  # Already in cache, be silent
  set(NETLINK_FIND_QUIETLY TRUE)
endif ()

libfind_pkg_check_modules(NETLINK_PKGCONF libnl-3.0)


find_path(NETLINK_INCLUDE_DIR netlink.h
	PATH ${NETLINK_PKGCONF_INCLUDE_DIRS}/netlink/)

find_library(NETLINK_LIBRARY
	NAMES nl-3
	PATH ${NETLINK_PKGCONF_LIBRARY_DIRS})

set(NETLINK_PROCESS_LIBS NETLINK_LIBRARY)

if (Netlink_FIND_COMPONENTS)
	foreach (component ${Netlink_FIND_COMPONENTS})
		string( TOUPPER ${component} COMPONENT)
		libfind_pkg_check_modules(NETLINK_${COMPONENT}_PKGCONF libnl-${component}-3.0)
		find_library(NETLINK_${COMPONENT}_LIBRARY
			NAMES nl-${component}-3
			PATHS ${NETLINK_${COMPONENT}_PKGCONF_LIBRARY_DIRS})
		set(NETLINK_PROCESS_LIBS ${NETLINK_PROCESS_LIBS} NETLINK_${COMPONENT}_LIBRARY)
	endforeach()
endif()


# Fuck yeah !
set(NETLINK_INCLUDE_DIR ${NETLINK_PKGCONF_INCLUDE_DIRS} CACHE PATH "Path to netlink" FORCE)
set(NETLINK_PROCESS_INCLUDES NETLINK_INCLUDE_DIR)
libfind_process(NETLINK)
