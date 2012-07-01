#include <QDebug>
#include "addcontactgui.h"
#include <QWidget>
#include "contact.h"
#include "clientgui.h"

addContactGui::addContactGui(Controller &controller, ClientGUI &win)
  : QWidget(0), _controller(controller),_view(win), _initialName("")
{
  this->setAttribute(Qt::WA_DeleteOnClose);
  setupUi(this);
  QObject::connect(okOrReject, SIGNAL(accepted()), &_controller, SLOT(addContact()));
  QObject::connect(okOrReject, SIGNAL(rejected()), &win, SLOT(deleteAddContact()));
}

addContactGui::addContactGui(Controller &controller, ClientGUI &win, const QString &name, const QString &key)
  : QWidget(0), _controller(controller),_view(win), _initialName(name)
{
    this->setAttribute(Qt::WA_DeleteOnClose);
    setupUi(this);
    QObject::connect(okOrReject, SIGNAL(accepted()), &_controller, SLOT(addContact()));
    QObject::connect(okOrReject, SIGNAL(rejected()), &win, SLOT(deleteAddContact()));
    this->name->setText(name);
    this->pubKey->setText(key);

}


addContactGui::~addContactGui()
{

}

QString addContactGui::getNewContactName() const
{
  return (this->name->text());
}

const QString addContactGui::getInitialContactName() const
{
  return this->_initialName;
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
