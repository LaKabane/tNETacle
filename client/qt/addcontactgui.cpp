#include "addcontactgui.h"
#include <QWidget>
#include "contact.h"
#include "clientgui.h"

addContactGui::addContactGui(ClientGUI* client, QWidget* parents)
    : QWidget(parents), _clientGui(client)
{
    setupUi(this);
    QObject::connect(okOrReject, SIGNAL(accepted()), this, SLOT(sendContact()));
}

addContactGui::~addContactGui()
{
}

void addContactGui::sendContact() {
    Contact* newContact = new Contact(this->nameLabel->text().toStdString(),
                                      this->keyText->toPlainText().toStdString());
    std::cout << "Lololilolilol" << *newContact << std::endl;
    _clientGui->addContact(newContact);
}
