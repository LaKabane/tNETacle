project(tNETacle)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAME_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/Modules/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(LINUX True)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(WINDOWS True)
endif()

#find_package(ZLIB REQUIRED)
find_package(Event REQUIRED)

set(SOURCES_LIST)
#file(GLOB SOURCES_LIST src/*.c)

if(UNIX)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall")

	file(GLOB UNIX_SOURCES_LIST sys/unix/*.c)
	set(SOURCES_LIST ${SOURCES_LIST} ${UNIX_SOURCES_LIST})

	if(LINUX)
	  add_definitions(-DLinux -D_GNU_SOURCE)
	endif()
endif()

if (WINDOWS)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})

	file(GLOB WIN_SOURCES_LIST sys/wnt/*.c)
	set(SOURCES_LIST ${SOURCES_LIST} ${WIN_SOURCES_LIST})
endif()

include_directories(
  ${CMAKE_HOME_DIRECTORY}/include
)

add_executable(tNETacle
	${SOURCES_LIST}
)

target_link_libraries(tNETacle
	#${ZLIB_LIBRARIES}
	${EVENT_LIBRARIES}
	)
