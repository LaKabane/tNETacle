SOURCES += \
    main.cpp \
    ../share/controler.cpp \
    ../share/contactsList.cpp \
    ../share/contact.cpp \
    ../share/config.cpp \
    clientgui.cpp \
    controller.cpp\
    addcontactgui.cpp \
    contactgui.cpp

FORMS += \
    client.ui \
    addContact.ui

RESOURCES += \
    ../share/img/client.qrc

HEADERS += \
    controller.h\
    clientgui.h \
    ../share/controler.h \
    ../share/contactsList.h \
    ../share/contact.h \
    ../share/config.h \
    addcontactgui.h \
    contactgui.h

INCLUDEPATH += ../share/
