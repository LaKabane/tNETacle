QT += network


LIBS +=  -lqjson

SOURCES += \
    main.cpp \
    contactsList.cpp \
    contact.cpp \
    config.cpp \
    clientgui.cpp \
    configgui.cpp \
    imodel.cpp \
    controller.cpp\
    addcontactgui.cpp \
    rootnodegui.cpp\
    modelcontact.cpp\
    modelconfig.cpp\
    modelrootnode.cpp\
    modellog.cpp\
    network.cpp

FORMS += \
    client.ui \
    config.ui \
    addcontact.ui \
    rootnode.ui

RESOURCES += \
    ../share/img/client.qrc

HEADERS += \
    include/exception.h\
    include/singleton.hpp\
    include/network.h\
    include/imodel.h    \
#    include/Registry.h    \
    include/modelcontact.h    \
    include/modelconfig.h    \
    include/modelrootnode.h    \
    include/modellog.h    \
    include/controller.h\
    include/clientgui.h \
    include/contactsList.h \
    include/contact.h \
    include/config.h \
    include/addcontactgui.h \
    include/configgui.h \
    include/rootnodegui.h \
    include/iclientgui.h

INCLUDEPATH += \
               ./include/
