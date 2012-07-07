#include <QDebug>
#include "addcontactgui.h"
#include <QWidget>
#include "contact.h"
#include "clientgui.h"

AddContactGui::AddContactGui(Controller &controller, ClientGUI &win)
  : QWidget(0), _controller(controller),_view(win), _initialName("")
{
  this->setAttribute(Qt::WA_DeleteOnClose);

  setupUi(this);
  QObject::connect(okOrReject, SIGNAL(accepted()), &_controller, SLOT(addContact()));
  QObject::connect(okOrReject, SIGNAL(rejected()), &win, SLOT(deleteAddContact()));
  QObject::connect(selectPubKey->button(QDialogButtonBox::Open), SIGNAL(clicked()), this, SLOT(openPubKey()));
}

AddContactGui::AddContactGui(Controller &controller, ClientGUI &win, const QString &name, const QString &key)
  : QWidget(0), _controller(controller),_view(win), _initialName(name)
{
    this->setAttribute(Qt::WA_DeleteOnClose);
    setupUi(this);
    QObject::connect(okOrReject, SIGNAL(accepted()), &_controller, SLOT(addContact()));
    QObject::connect(okOrReject, SIGNAL(rejected()), &win, SLOT(deleteAddContact()));
    this->name->setText(name);
    this->pubKey->setText(key);
}


AddContactGui::~AddContactGui()
{
}

QString AddContactGui::getNewContactName() const
{
  return (this->name->text());
}

const QString AddContactGui::getInitialContactName() const
{
  return this->_initialName;
}

QString AddContactGui::getNewContactKey() const
{
  return (this->pubKey->toPlainText());
}


void AddContactGui::sendContact() {
    // Contact* newContact = new Contact(this->nameLabel->text().toStdString(),
    //                                   this->keyText->toPlainText().toStdString());
    // _clientGui->addContact(newContact);
}

void	AddContactGui::openPubKey()
{
  this->pubKey->setText(_controller.openPubKey());
}
