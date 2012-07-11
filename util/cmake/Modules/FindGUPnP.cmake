# Find Libgupnp
#
# Once done, this will define:
#
#  GUPnP_FOUND - system has GUPnP
#  GUPnP_INCLUDE_DIRS - the GUPnP include directories
#  GUPnP_LIBRARIES - link these to use GUPnP
#

include(LibFindMacros)

libfind_package(GUPnP GObject)
libfind_package(GUPnP GThread)

if (GUPnP_FIND_COMPONENTS)
	foreach( component ${GUPnP_FIND_COMPONENTS})
		string( TOUPPER ${component} _COMPONENT)
		set(GUPnP_USE_${_COMPONENT} True)
	endforeach(component)

        if (${GUPnP_USE_CORE})
                libfind_pkg_check_modules(GUPnP_CORE_PKGCONF gupnp-1.0)
                find_path(GUPnP_CORE_INCLUDE_DIR
                    NAMES libgupnp/gupnp.h
                    PATHS ${GUPnP_CORE_PKGCONF_INCLUDE_DIRS}
                    PATHS ${GObject_INCLUDE_DIRS}
                )

		find_library(GUPnP_CORE_LIBRARY
			NAMES gupnp-1.0
                        PATHS ${GUPnP_CORE_PKGCONF_LIBRARY_DIRS}
                )
	endif()

        if (${GUPnP_USE_IGD})
            libfind_pkg_check_modules(GUPnP_IGD_PKGCONF gupnp-igd-1.0)
            find_path(GUPnP_IGD_INCLUDE_DIR
                NAMES libgupnp-igd/gupnp-simple-igd.h
                PATHS ${GUPnP_IGD_PKGCONF_INCLUDE_DIRS}
                PATHS ${GObject_INCLUDE_DIRS}
            )

            find_library(GUPnP_IGD_LIBRARY
		NAMES gupnp-igd-1.0
                PATHS ${GUPnP_IGD_PKGCONF_LIBRARY_DIRS}
            )
	endif()

        set(GUPnP_INCLUDE_DIRS ${GUPnP_IGD_INCLUDE_DIR} ${GUPnP_CORE_INCLUDE_DIR})
        set(GUPnP_LIBRARIES ${GUPnP_IGD_LIBRARY} ${GUPnP_CORE_LIBRARY})

else()
        libfind_pkg_check_modules(GUPnP_PKGCONF gupnp-1.0)

        find_path(GUPnP_INCLUDE_DIRS libgupnp/gupnp.h
            PATHS ${GUPnP_PKGCONF_LIBRARY_DIRS}
            PATHS ${GObject_INCLUDE_DIRS}
        )

        find_library(GUPnP_LIBRARY
                NAMES gupnp-1.0
                PATHS ${GUPnP_PKGCONF_LIBRARY_DIRS}
        )
	set(GUPnP_LIBRARIES ${GUPnP_LIBRARY})

endif()

set(GUPnP_PROCESS_INCLUDES GUPnP_INCLUDE_DIRS GObject_INCLUDE_DIRS GThread_INCLUDE_DIRS)
set(GUPnP_PROCESS_LIBS GUPnP_LIBRARIES GObject_LIBRARIES GThread_LIBRARIES)
libfind_process(GUPnP)
