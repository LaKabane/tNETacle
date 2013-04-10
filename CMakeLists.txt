# tNETacle CMakeLists.txt
# ========================

project(tNETacle)
cmake_minimum_required(VERSION 2.8)

# CMake Configuration
# ------------------
set(CMAKE_MODULE_PATH ${CMAME_MODULE_PATH} ${CMAKE_SOURCE_DIR}/util/cmake/Modules/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(${CMAKE_SYSTEM_NAME} True)

# If you need some custom, site local specific modification, use this file.
include(${CMAKE_SOURCE_DIR}/CMakeLists.txt.local OPTIONAL)

# CMake global options
# --------------------
# None at the moment

# Global Packages search
# ----------------------
find_package(YAJL REQUIRED)
find_package(TUNTAP)
find_package(OpenSSL REQUIRED)
find_package(TCLT REQUIRED)
if(WIN32)
    set(EVENT_COMPONENTS_LIST core)
else()
    set(EVENT_COMPONENTS_LIST core openssl)
endif()
find_package(Event COMPONENTS ${EVENT_COMPONENTS_LIST} REQUIRED)
if(NOT TUNTAP_FOUND)
    find_package(Tapcfg REQUIRED)
endif()

# Global CPP definitions
# ----------------------
add_definitions(-D${CMAKE_SYSTEM_NAME})
add_definitions(-DUSE_TCLT)
if(TUNTAP_FOUND)
    add_definitions(-DUSE_LIBTUNTAP)
elseif(TAPCFG_FOUND)
    add_definitions(-DUSE_TAPCFG)
endif()

# Global include directories
# --------------------------
include_directories(${CMAKE_HOME_DIRECTORY}/include)
include_directories(${EVENT_INCLUDE_DIR})
include_directories(${YAJL_INCLUDE_DIR})
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${TCLT_INCLUDE_DIR})

if(${CALM_INCLUDE_DIR})
    include_directories(${CALM_INCLUDE_DIR})
else()
    set(CALM_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/sub/calm-containers/include)
    include_directories(${CALM_INCLUDE_DIR})
endif()

if(TUNTAP_FOUND)
    include_directories(${TUNTAP_INCLUDE_DIRS})
else()
    include_directories(${TAPCFG_INCLUDE_DIRS})
endif()

# Portable source files
# ---------------------
set(SOURCES_LIST
    src/conf.c
    src/tun-compat.c
    #src/compress.c
    #src/pipeline.c
    src/mc.c
    src/hexdump.c
    src/server.c
    src/udp.c
    src/device.c
    src/frame.c
    src/coro.c
    src/client.c
    src/sched.c
    src/dtls.c
    src/subset.c
    src/endpoint.c
    src/log_extern.c
    src/utility.c
)

set(HEADERS_LIST 
    include/compress.h
    include/hexdump.h
    include/log.h
    include/mc.h
    include/options.h
    include/pathnames.h
    include/server.h
    include/tnetacle.h
    include/tntexits.h
    include/tntsocket.h
    include/tun.h
    ${CALM_INCLUDE_DIR}/map.h
    ${CALM_INCLUDE_DIR}/vector.h
    include/client.h
    include/imsg.h
    include/wincompat.h
    include/udp.h
    include/device.h
    include/frame.h
    include/coro.h
    include/tntsched.h
    include/dtls.h
    include/subset.h
    include/endpoint.h
    include/log_extern.h
    include/utility.h
)

# OS families specific things
# ---------------------------
if(UNIX)
    # Unix specific include directories
    # ---------------------------------
    include_directories(/usr/include/)
    
    # Unix specific definitions
    # -------------------------
    add_definitions(-DUnix)
    
    # Unix specific source files
    # --------------------------
    set(SOURCES_LIST ${SOURCES_LIST} ${HEADERS_LIST}
        sys/unix/log.c
        sys/unix/imsg.c
        sys/unix/imsg-buffer.c
        sys/unix/tnetacle.c
        sys/unix/tnetacled.c
        sys/unix/util.c
        sys/unix/conf.c
        sys/unix/tntsocket.c
    )
endif()

if(Windows)
    # CMake windows Configuration
    # ---------------------------
    set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})
    add_definitions(-DWindows)
    add_definitions(-DCORO_WIN32)
    
    # Windows source files
    # --------------------
    set(SOURCES_LIST ${SOURCES_LIST} ${HEADERS_LIST}
        sys/wnt/conf.c
        sys/wnt/log.c
        sys/wnt/tntsocket.c
        sys/wnt/winstrtonum.c
        sys/wnt/main.c
        sys/wnt/utils.c
    )
endif()

# OS specific things
# ------------------
if(UNIX)
if(Linux)
    set(CMAKE_SYSTEM_INCLUDE_PATH
      ${CMAKE_SYSTEM_INCLUDE_PATH} /usr/local/include)
    option(ENABLE_BSDCOMPAT "Enable the use of the libbsd" OFF)
    include_directories(/usr/local/include)

    add_definitions(-DCORO_SJLJ)
    add_definitions(-D_GNU_SOURCE)
    add_definitions(-DHAVE_SETRESXID)
    add_definitions(-DHAVE_WORKINGKQUEUE)
    if(ENABLE_BSDCOMPAT)
        find_package(BSD)
        if(BSD_FOUND)
            add_definitions(-DHAVE_BSD_COMPAT)
            add_definitions(-DHAVE_SETPROCTITLE)
            include_directories(${BSD_INCLUDE_DIRS})
        endif(BSD_FOUND)
    endif(ENABLE_BSDCOMPAT)
elseif(OpenBSD)
    set(CMAKE_SYSTEM_INCLUDE_PATH
      ${CMAKE_SYSTEM_INCLUDE_PATH} /usr/local/include)
    include_directories(/usr/local/include)

    add_definitions(-DCORO_SJLJ)
    add_definitions(-DHAVE_SETPROCTITLE)
    add_definitions(-DHAVE_SETRESXID)
    add_definitions(-DHAVE_WORKINGKQUEUE)
elseif(NetBSD)
    set(CMAKE_SYSTEM_INCLUDE_PATH
      ${CMAKE_SYSTEM_INCLUDE_PATH} /usr/pkg/include)
    include_directories(/usr/pkg/include)

    add_definitions(-DCORO_SJLJ)
    add_definitions(-DHAVE_SETPROCTITLE)
    add_definitions(-DHAVE_WORKINGKQUEUE)
elseif(FreeBSD)
    set(CMAKE_SYSTEM_INCLUDE_PATH
      ${CMAKE_SYSTEM_INCLUDE_PATH} /usr/local/include)
    include_directories(/usr/local/include)

    add_definitions(-DCORO_SJLJ)
    add_definitions(-DHAVE_SETPROCTITLE)
    add_definitions(-DHAVE_SETRESXID)
    add_definitions(-DHAVE_WORKINGKQUEUE)
elseif(Darwin)
    set(CMAKE_SYSTEM_INCLUDE_PATH
      ${CMAKE_SYSTEM_INCLUDE_PATH} /opt/local/include)
    include_directories(/opt/local/include)

    add_definitions(-DCORO_SJLJ)
else()
    message(FATAL_ERROR "Your operating system is not supported yet")
endif()
endif(UNIX)

# Executable definitons
# ---------------------
add_executable(tNETacle ${SOURCES_LIST})

# Global linked libraries
# -----------------------
target_link_libraries(tNETacle
    ${EVENT_LIBRARIES}
    ${YAJL_LIBRARIES}
    ${OPENSSL_LIBRARIES}
)

if(TUNTAP_FOUND)
    target_link_libraries(tNETacle ${TUNTAP_LIBRARIES})
elseif(TAPCFG_FOUND)
    target_link_libraries(tNETacle ${TAPCFG_LIBRARIES})
endif()

target_link_libraries(tNETacle ${TCLT_LIBRARY})

# Linux linked libraries
# ----------------------
if(ENABLE_BSDCOMPAT AND BSD_FOUND)
    target_link_libraries(tNETacle ${BSD_LIBRARIES})
endif()

# Windows linked libraries
# ------------------------
if(Windows)
    target_link_libraries(tNETacle ws2_32.lib)
endif()

