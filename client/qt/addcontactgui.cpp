#include "addcontactgui.h"
#include <QWidget>
#include "contact.h"
#include "clientgui.h"

addContactGui::addContactGui(Controller &controller, QWidget* parents)
    : QWidget(parents), _controller(controller)
{
    setupUi(this);
    QObject::connect(okOrReject, SIGNAL(accepted()), &_controller, SLOT(addContact()));
}

addContactGui::~addContactGui()
{
}

QString addContactGui::getNewContactName() const
{
  return (this->name->text());
}

QString addContactGui::getNewContactKey() const
{
  return (this->pubKey->toPlainText());
}


void addContactGui::sendContact() {
    // Contact* newContact = new Contact(this->nameLabel->text().toStdString(),
    //                                   this->keyText->toPlainText().toStdString());
    // std::cout << "Lololilolilol" << *newContact << std::endl;
    // _clientGui->addContact(newContact);
}
