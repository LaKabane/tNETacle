project(tNETacle)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

add_definitions(-D${CMAKE_SYSTEM_NAME})
add_definitions(-DHAVE_SETPROCTITLE)

if(UNIX)
  set(CMAKE_C_FLAGS "-W -Wall")
  file(GLOB SOURCES_LIST sys/unix/log.c)
  file(GLOB SOURCES_LIST sys/unix/imsg.c)
  file(GLOB SOURCES_LIST sys/unix/imsg-buffer.c)
  file(GLOB SOURCES_LIST sys/unix/tnetacle.c)
  file(GLOB SOURCES_LIST sys/unix/tnetacled.c)
  file(GLOB SOURCES_LIST sys/unix/util.c)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(LINUX True)
  add_definitions(-D_GNU_SOURCE)
  file(GLOB SOURCES_LIST sys/unix/tun-linux.c)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD" OR "FreeBSD" OR "NetBSD")
  file(GLOB SOURCES_LIST sys/unix/tun-bsd.c)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  remove_definitions(-DHAVE_SETPROCTITLE)
endif()

if(USE_CLANG)
  set(CMAKE_C_COMPILER clang)
endif(USE_CLANG)

include_directories(
  ${CMAKE_HOME_DIRECTORY}/include
)

add_executable(tNETacle
	${SOURCES_LIST}
)

