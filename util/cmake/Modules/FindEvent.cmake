# Find Libevent
# http://monkey.org/~provos/libevent/
#
# Once done, this will define:
#
#  Event_FOUND - system has Event
#  Event_INCLUDE_DIRS - the Event include directories
#  Event_LIBRARIES - link these to use Event
#

if (EVENT_INCLUDE_DIR AND EVENT_LIBRARY)
  # Already in cache, be silent
  set(EVENT_FIND_QUIETLY TRUE)
endif (EVENT_INCLUDE_DIR AND EVENT_LIBRARY)

find_path(EVENT_INCLUDE_DIR event.h
  PATH_SUFFIXES event2
)

if (Event_FIND_COMPONENTS)
	foreach( component ${Event_FIND_COMPONENTS})
		string( TOUPPER ${component} _COMPONENT)
		set(EVENT_USE_${_COMPONENT} True)
	endforeach(component)

	if (${EVENT_USE_CORE})
		find_library(EVENT_CORE_LIBRARY
			NAMES event_core)
		if (${EVENT_CORE_LIBRARY} MATCHES "EVENT_CORE_LIBRARY-NOTFOUND" AND Event_FIND_REQUIRED)
			message(STATUS "Event core not found.")
		endif()
		if (NOT ${EVENT_FIND_QUIETLY})
			message(STATUS "Event core found at ${EVENT_CORE_LIBRARY}.")
		endif()
	endif()

	if (${EVENT_USE_OPENSSL})
		find_library(EVENT_OPENSSL_LIBRARY
			NAMES event_openssl)
		if (${EVENT_OPENSSL_LIBRARY} MATCHES "EVENT_OPENSSL_LIBRARY-NOTFOUND" AND Event_FIND_REQUIRED)
			message(STATUS "Event openssl module not found.")
		endif()
		if (NOT ${EVENT_FIND_QUIETLY})
			message(STATUS "Event openssl module found at ${EVENT_OPENSSL_LIBRARY}.")
		endif()
	endif()

	if (${EVENT_USE_PTHREADS})
		find_library(EVENT_PTHREADS_LIBRARY
			NAMES event_pthreads)
		if (${EVENT_PTHREADS_LIBRARY} MATCHES "EVENT_PTHREADS_LIBRARY-NOTFOUND" AND Event_FIND_REQUIRED)
			message(STATUS "Event pthreads module not found.")
		endif()
		if (NOT ${EVENT_FIND_QUIETLY})
			message(STATUS "Event pthreads module found at ${EVENT_PTHREADS_LIBRARY}.")
		endif()
	endif()

	if (${EVENT_USE_EXTRA})
		find_library(EVENT_EXTRA_LIBRARY
			NAMES event_extra)
		if (${EVENT_EXTRA_LIBRARY} MATCHES "EVENT_EXTRA_LIBRARY-NOTFOUND" AND Event_FIND_REQUIRED)
			message(STATUS "Event extra module not found.")
		endif()
		if (NOT ${EVENT_FIND_QUIETLY})
			message(STATUS "Event extra module found at ${EVENT_EXTRA_LIBRARY}.")
		endif()
	endif()

	set(EVENT_LIBRARIES 	${EVENT_OPENSSL_LIBRARY} 
		${EVENT_CORE_LIBRARY}
		${EVENT_PTHREADS_LIBRARY}
		${EVENT_EXTRA_LIBRARY})

else()
	find_library(EVENT_LIBRARY
		NAMES event)

	set(EVENT_LIBRARIES ${EVENT_LIBRARY})

endif()
set(EVENT_INCLUDE_DIRS ${EVENT_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(EVENT
  DEFAULT_MSG
  EVENT_INCLUDE_DIR
  EVENT_LIBRARIES
)

mark_as_advanced(EVENT_INCLUDE_DIR EVENT_LIBRARY
		EVENT_CORE_LIBRARY
		EVENT_OPENSSL_LIBRARY
		EVENT_PTHREADS_LIBRARY
		EVENT_EXTRA_LIBRARY)
