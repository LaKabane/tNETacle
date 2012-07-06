QT += network


LIBS +=  -lqjson

SOURCES += \
    main.cpp \
    contactsList.cpp \
    contact.cpp \
    config.cpp \
    clientgui.cpp \
    model.cpp \
    controller.cpp\
    addcontactgui.cpp \
    contactgui.cpp \
    rootnodegui.cpp\
    modelcontacts.cpp\
    modellog.cpp\
    network.cpp

FORMS += \
    client.ui \
    addContact.ui \
    rootNode.ui

RESOURCES += \
    ../share/img/client.qrc

HEADERS += \
    include/exception.h\
    include/singleton.hpp\
    include/network.h\
    include/Model.h    \
    include/Registry.h    \
    include/modelcontacts.h    \
    include/modellog.h    \
    include/controller.h\
    include/clientgui.h \
    include/contactsList.h \
    include/contact.h \
    include/config.h \
    include/addcontactgui.h \
    include/contactgui.h \
    include/rootnodegui.h

INCLUDEPATH += ../share/ \
               ./include/
