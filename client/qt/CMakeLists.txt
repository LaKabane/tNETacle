# tNETacle CMakeLists.txt
# ========================

cmake_minimum_required(VERSION 2.6)
PROJECT(client)
FIND_PACKAGE(Qt4 REQUIRED)
FIND_PACKAGE(Tclt REQUIRED)

SET(QT_USE_QTNETWORK TRUE)
INCLUDE(${QT_USE_FILE})

SET(client_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/config.cpp
    ${CMAKE_CURRENT_LIST_DIR}/clientgui.cpp
    ${CMAKE_CURRENT_LIST_DIR}/configgui.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/controller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/addcontactgui.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rootnodegui.cpp
    ${CMAKE_CURRENT_LIST_DIR}/modelcontact.cpp
    ${CMAKE_CURRENT_LIST_DIR}/modelconfig.cpp
    ${CMAKE_CURRENT_LIST_DIR}/modelrootnode.cpp
    ${CMAKE_CURRENT_LIST_DIR}/modellog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/protocol.cpp
    ${CMAKE_CURRENT_LIST_DIR}/network.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils.cpp
)

SET(client_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/include/network.h
    ${CMAKE_CURRENT_LIST_DIR}/include/controller.h
    ${CMAKE_CURRENT_LIST_DIR}/include/clientgui.h
    ${CMAKE_CURRENT_LIST_DIR}/include/addcontactgui.h
    ${CMAKE_CURRENT_LIST_DIR}/include/configgui.h
    ${CMAKE_CURRENT_LIST_DIR}/include/rootnodegui.h
)

SET(client_FORMS
    ${CMAKE_CURRENT_LIST_DIR}/client.ui
    ${CMAKE_CURRENT_LIST_DIR}/config.ui
    ${CMAKE_CURRENT_LIST_DIR}/addcontact.ui
    ${CMAKE_CURRENT_LIST_DIR}/rootnode.ui
)

SET(client_RESOURCES
    ${CMAKE_CURRENT_LIST_DIR}/../share/img/client.qrc
)

message(${TCLT_LIBRARY})
SET(client_LIBRARIES ${QT_LIBRARIES} ${TCLT_LIBRARY})

QT4_WRAP_CPP(client_HEADERS_MOC ${client_HEADERS})
QT4_WRAP_UI(client_FORMS_HEADERS ${client_FORMS})
QT4_ADD_RESOURCES(client_RESOURCES_RCC ${client_RESOURCES})

ADD_DEFINITIONS(${QT_DEFINITIONS})

ADD_EXECUTABLE(client ${client_SOURCES} 
    ${client_HEADERS_MOC}
    ${client_FORMS_HEADERS}
    ${client_RESOURCES_RCC}
)

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${TCLT_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(client ${client_LIBRARIES})
