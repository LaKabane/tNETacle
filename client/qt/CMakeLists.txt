# tNETacle CMakeLists.txt
# ========================

cmake_minimum_required(VERSION 2.6)
PROJECT(QtNETacle)
FIND_PACKAGE(Qt4 REQUIRED)

SET(QT_USE_QTNETWORK TRUE)
INCLUDE(${QT_USE_FILE})

SET(client_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/config.cpp
    ${CMAKE_CURRENT_LIST_DIR}/qclient.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/controller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/modelcontact.cpp
    ${CMAKE_CURRENT_LIST_DIR}/modelconfig.cpp
    ${CMAKE_CURRENT_LIST_DIR}/modelconnexion.cpp
    ${CMAKE_CURRENT_LIST_DIR}/modelrootnode.cpp
    ${CMAKE_CURRENT_LIST_DIR}/modellog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/protocol.cpp
    ${CMAKE_CURRENT_LIST_DIR}/network.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/theader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bodyconnexion.cpp
)

SET(client_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/include/network.h
    ${CMAKE_CURRENT_LIST_DIR}/include/controller.h
    ${CMAKE_CURRENT_LIST_DIR}/include/qclient.h
    ${CMAKE_CURRENT_LIST_DIR}/include/theader.h
    ${CMAKE_CURRENT_LIST_DIR}/include/bodyconnexion.h
    ${CMAKE_CURRENT_LIST_DIR}/include/imodel.h
    ${CMAKE_CURRENT_LIST_DIR}/include/controller.h
    ${CMAKE_CURRENT_LIST_DIR}/include/modelcontact.h
    ${CMAKE_CURRENT_LIST_DIR}/include/modelconfig.h
    ${CMAKE_CURRENT_LIST_DIR}/include/modelconnexion.h
    ${CMAKE_CURRENT_LIST_DIR}/include/modelrootnode.h
    ${CMAKE_CURRENT_LIST_DIR}/include/modellog.h
    ${CMAKE_CURRENT_LIST_DIR}/include/protocol.h
    ${CMAKE_CURRENT_LIST_DIR}/include/network.h
    ${CMAKE_CURRENT_LIST_DIR}/include/utils.h
)

SET(client_FORMS
    ${CMAKE_CURRENT_LIST_DIR}/qclient.ui
    ${CMAKE_CURRENT_LIST_DIR}/theader.ui
    ${CMAKE_CURRENT_LIST_DIR}/bodymain.ui
    ${CMAKE_CURRENT_LIST_DIR}/bodyaddgroup.ui
    ${CMAKE_CURRENT_LIST_DIR}/bodyconnexion.ui
)

SET(client_RESOURCES
    ${CMAKE_CURRENT_LIST_DIR}/../share/img/client.qrc
)

SET(client_LIBRARIES ${QT_LIBRARIES} ${TCLT_LIBRARY})

QT4_WRAP_CPP(client_HEADERS_MOC ${client_HEADERS})
QT4_WRAP_UI(client_FORMS_HEADERS ${client_FORMS})
QT4_ADD_RESOURCES(client_RESOURCES_RCC ${client_RESOURCES})

ADD_DEFINITIONS(${QT_DEFINITIONS})

ADD_EXECUTABLE(QtNETacle ${client_SOURCES}
    ${client_HEADERS}
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

TARGET_LINK_LIBRARIES(QtNETacle ${client_LIBRARIES})
