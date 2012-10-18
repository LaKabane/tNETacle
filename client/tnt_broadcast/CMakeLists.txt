# tNETacle CMakeLists.txt
# ========================

cmake_minimum_required(VERSION 2.6)
PROJECT(tnt_broadcast)

find_package(Boost REQUIRED COMPONENTS system program_options)

SET(tnt_broadcast_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)

SET(tnt_broadcast_HEADERS
)

SET(tnt_broadcast_RESOURCES
    ${CMAKE_CURRENT_LIST_DIR}/../share/img/tnt_broadcast.qrc
)

SET(tnt_broadcast_LIBRARIES ${TCLT_LIBRARY})

ADD_EXECUTABLE(tnt_broadcast ${tnt_broadcast_SOURCES} 
    ${tnt_broadcast_HEADERS}
)

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${TCLT_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(tnt_broadcast ${tnt_broadcast_LIBRARIES} ${Boost_LIBRARIES})
