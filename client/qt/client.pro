QT += network


LIBS +=  -lqjson

SOURCES += \
    main.cpp \
#    ../share/controler.cpp \
    ../share/contactsList.cpp \
    ../share/contact.cpp \
    ../share/config.cpp \
    clientgui.cpp \
    model.cpp \
    controller.cpp\
    addcontactgui.cpp \
    contactgui.cpp \
    rootnodegui.cpp\
    model_contacts.cpp\
    model_log.cpp\
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
    include/model_contacts.h    \
    include/model_log.h    \
    include/controller.h\
    include/clientgui.h \
 #   ../share/controler.h \
    ../share/contactsList.h \
    ../share/contact.h \
    ../share/config.h \
    include/addcontactgui.h \
    include/contactgui.h \
    include/rootnodegui.h

INCLUDEPATH += ../share/ \
               ./include/
