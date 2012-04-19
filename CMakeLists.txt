project(tNETacle)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAME_MODULE_PATH} ${CMAKE_SOURCE_DIR}/util/cmake/Modules/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

set(${CMAKE_SYSTEM_NAME} True)

#find_package(ZLIB REQUIRED)
find_package(Event COMPONENTS core REQUIRED)
find_package(Tapcfg REQUIRED)
find_package(Tuntap REQUIRED)

# Global CPP definitions
add_definitions(-D${CMAKE_SYSTEM_NAME})
add_definitions(-DHAVE_SETPROCTITLE)
add_definitions(-DHAVE_SETRESXID)

# Portable source files
set(SOURCES_LIST
  src/conf.c
  src/tun-compat.c)

if (Linux)
	option(ENABLE_BSDCOMPAT "Enable the use of the libbsd" ON)
	option(ENABLE_NETLINK "Enable the use of libnl to configure the
	interfaces" ON)
endif()

if(UNIX)
  include_directories(/usr/include/ /usr/local/include)
  set(CMAKE_C_FLAGS "-W -Wall")
  set(SOURCES_LIST ${SOURCES_LIST} sys/unix/log.c
	  sys/unix/imsg.c sys/unix/imsg-buffer.c
	  sys/unix/tnetacle.c sys/unix/tnetacled.c
	  sys/unix/util.c sys/unix/server.c
	  sys/unix/conf.c)
endif()

if (WINDOWS)
  add_definitions(-DUSE_TAPCFG)

  set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})

  file(GLOB WIN_SOURCES_LIST sys/wnt/*.c)
  set(SOURCES_LIST ${SOURCES_LIST} ${WIN_SOURCES_LIST})
endif()

if(Linux)
  if (ENABLE_BSDCOMPAT)
    find_package(Bsd)
    if (BSD_FOUND)
      add_definitions(-DHAVE_BSD_COMPAT)
    else()
      remove_definitions(-DHAVE_SETPROCTITLE)
    endif()
  else()
    remove_definitions(-DHAVE_SETPROCTITLE)
  endif()

  if (ENABLE_NETLINK)
    find_package(Netlink COMPONENTS route)
    if (NETLINK_FOUND)
      include_directories(${NETLINK_INCLUDE_DIRS})
      add_definitions(-DHAVE_NETLINK)
    endif()
  endif()

  add_definitions(-DUSE_LIBTUNTAP)

  add_definitions(-D_GNU_SOURCE)
endif()

if (OpenBSD)
  add_definitions(-DUSE_LIBTUNTAP)
endif()

if (NetBSD)
  add_definitions(-DUSE_TAPCFG)
endif()

if (FreeBSD)
  add_definitions(-DUSE_TAPCFG)
endif()

if (Darwin)
  add_definitions(-DUSE_TAPCFG)
  remove_definitions(-DHAVE_SETPROCTITLE)
  remove_definitions(-DHAVE_SETRESXID)
#macports
  include_directories(/opt/local/include)
endif()

include_directories(
${CMAKE_HOME_DIRECTORY}/include
)

add_executable(tNETacle
	${SOURCES_LIST}
)

if (TAPCFG_FOUND)
  target_link_libraries(tNETacle ${TAPCFG_LIBRARIES})
elseif(TUNTAP_FOUND)
  target_link_libraries(tNETacle ${TUNTAP_LIBRARIES})
endif()

if (Linux)
  if (ENABLE_BSDCOMPAT AND BSD_FOUND)
      target_link_libraries(tNETacle ${BSD_LIBRARIES})
  endif()

  if (ENABLE_NETLINK AND NETLINK_FOUND)
      target_link_libraries(tNETacle ${NETLINK_LIBRARIES})
  endif()
endif()

target_link_libraries(tNETacle
	${EVENT_LIBRARIES})
