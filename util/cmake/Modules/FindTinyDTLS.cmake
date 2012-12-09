# Find tinydtls
#
# Once done, this will define:
#
#  TINYDTLS_FOUND - system has tinydtls compat
#  TINYDTLS_INCLUDE_DIRS - the tinydtls compat include directories
#  TINYDTLS_LIBRARIES - link these to use tinydtls compat
#

include(LibFindMacros)

if (TinyDTLS_FIND_REQUIRED)
    set(TINYDTLS_FIND_REQUIRED)
endif()

libfind_pkg_check_modules(TINYDTLS_PKGCONF libtinydtls)

find_path(TINYDTLS_INCLUDE_DIR dtls.h
  PATH ${TINYDTLS_PKGCONF_INCLUDE_DIRS}
)

find_library(TINYDTLS_LIBRARY tinydtls)

set(TINYDTLS_PROCESS_INCLUDES TINYDTLS_INCLUDE_DIR)
set(TINYDTLS_PROCESS_LIBS TINYDTLS_LIBRARY)
libfind_process(TINYDTLS)
