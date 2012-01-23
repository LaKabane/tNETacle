project(tNETacle)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(LINUX True)
  add_definitions(-DLinux
	  -D_GNU_SOURCE
	  )
endif()

if(UNIX)
	set(CMAKE_C_FLAGS "-W -Wall")
	file(GLOB SOURCES_LIST sys/unix/*.c)
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

