# tnt broadcast CMakeLists.txt
# ============================

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(tnt_broadcast)

FIND_PACKAGE(Boost REQUIRED COMPONENTS system program_options)

SET(CMAKE_CXX_FLAGS "-std=c++11")

SET(tnt_broadcast_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)

#SET(tnt_broadcast_RESOURCES
#    ${CMAKE_CURRENT_LIST_DIR}/../share/img/tnt_broadcast.qrc
#)

SET(tnt_broadcast_LIBRARIES ${TCLT_LIBRARY})

ADD_EXECUTABLE(tnt_broadcast ${tnt_broadcast_SOURCES})

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${TCLT_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(tnt_broadcast
    ${tnt_broadcast_LIBRARIES}
    ${Boost_LIBRARIES}
)
