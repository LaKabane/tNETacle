SOURCES += \
    main.cpp \
    ../cli/controler.cpp \
    ../cli/contactsList.cpp \
    ../cli/contact.cpp \
    ../cli/config.cpp \
    clientgui.cpp \
    addcontactgui.cpp \
    contactgui.cpp

FORMS += \
    client.ui \
    addContact.ui

RESOURCES += \
    ../share/client.qrc

HEADERS += \
    clientgui.h \
    ../cli/controler.h \
    ../cli/contactsList.h \
    ../cli/contact.h \
    ../cli/config.h \
    addcontactgui.h \
    contactgui.h

INCLUDEPATH += ../cli/
