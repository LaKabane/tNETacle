SOURCES += \
    main.cpp \
#    ../share/controler.cpp \
    ../share/contactsList.cpp \
    ../share/contact.cpp \
    ../share/config.cpp \
    clientgui.cpp \
    controller.cpp\
    addcontactgui.cpp \
    contactgui.cpp \
    rootnodegui.cpp\
    model_contacts.cpp

FORMS += \
    client.ui \
    addContact.ui \
    rootNode.ui

RESOURCES += \
    ../share/img/client.qrc

HEADERS += \
    include/IModel.h    \
    include/model_contacts.h    \
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
