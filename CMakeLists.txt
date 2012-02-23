project(tNETacle)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAME_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/Modules/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)


set(${CMAKE_SYSTEM_NAME} True)

#find_package(ZLIB REQUIRED)
find_package(Event REQUIRED)

add_definitions(-D${CMAKE_SYSTEM_NAME})
add_definitions(-DHAVE_SETPROCTITLE)

if(UNIX)
  add_definitions(-DTNETACLE_LOCAL_ADDR=\"1.2.3.4\")
  set(CMAKE_C_FLAGS "-W -Wall")
  set(SOURCES_LIST ${SOURCES_LIST} sys/unix/log.c
	  sys/unix/imsg.c sys/unix/imsg-buffer.c
	  sys/unix/tnetacle.c sys/unix/tnetacled.c
	  sys/unix/util.c)
endif()

if (WINDOWS)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})

  file(GLOB WIN_SOURCES_LIST sys/wnt/*.c)
  set(SOURCES_LIST ${SOURCES_LIST} ${WIN_SOURCES_LIST})
endif()

if(Linux)
  add_definitions(-D_GNU_SOURCE)
  set(SOURCES_LIST ${SOURCES_LIST} sys/unix/tun-linux.c)
endif()

if (OpenBSD OR FreeBSD OR NetBSD)
  set(SOURCES_LIST ${SOURCES_LIST} sys/unix/tun-bsd.c)
endif()

if (Darwin)
  remove_definitions(-DHAVE_SETPROCTITLE)
endif()

include_directories(
${CMAKE_HOME_DIRECTORY}/include
)

add_executable(tNETacle
	${SOURCES_LIST}
)

# If we are on Linux, try to find libbsd-compat and add it the the link
# dependencies. The ENABLE_BSDCOMPAT is here to prevent from automagic
# dependencies.
if (Linux)
	option(ENABLE_BSDCOMPAT "Enable the use of the libbsd" ON)
endif()

if (Linux)
  if (ENABLE_BSDCOMPAT)
    find_package(Bsd)
    if (BSD_FOUND)
      target_link_libraries(tNETacle ${BSD_LIBRARIES})
      add_definitions(-DHAVE_BSD_COMPAT)
    else()
      remove_definitions(-DHAVE_SETPROCTITLE)
    endif()
  else()
    remove_definitions(-DHAVE_SETPROCTITLE)
  endif()
endif()

target_link_libraries(tNETacle
	${EVENT_LIBRARIES})
