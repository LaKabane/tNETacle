#include "clientgui.h"
#include <iostream>
#include "addcontactgui.h"

ClientGUI::ClientGUI(QMainWindow *parent) :
    QMainWindow(parent)
{
   setupUi(this);
   QObject::connect(actionAddContact, SIGNAL(activated()), this, SLOT(createAddContact()));
}

ClientGUI::~ClientGUI() {
}

void ClientGUI::createAddContact() {
    _addContact = new addContactGui(this, 0);

    QObject::connect(_addContact, SIGNAL(destroyed()), this, SLOT(deleteAddContact()));
    _addContact->show();
}

void ClientGUI::deleteAddContact() {
    delete _addContact;
    _addContact = 0;
}

void ClientGUI::addContact(Contact* c) {
    QString str;
    str.append(c->getName().c_str());
    this->ContactsList->addItem(str);
    _controler->addContact(c);
}
