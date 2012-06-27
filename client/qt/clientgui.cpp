#include <QDebug>
#include "clientgui.h"
#include <iostream>
#include "addcontactgui.h"
#include "rootnodegui.h"

ClientGUI::ClientGUI(QMainWindow *parent) :
    QMainWindow(parent),
    _addContact(0),
    _rootNode(0),
    _controller(*this)
{
   setupUi(this);
   QObject::connect(actionAddContact, SIGNAL(activated()), this, SLOT(createAddContact()));
   QObject::connect(actionDeleteContact, SIGNAL(activated()), &_controller, SLOT(deleteContact()));
   QObject::connect(ContactsList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), &_controller, SLOT(editContact(QListWidgetItem *)));

   QObject::connect(actionChangeRoot, SIGNAL(activated()), &_controller, SLOT(editRootNode()));
}

ClientGUI::~ClientGUI() {
}

void    ClientGUI::deleteSelected()
{
  int pos = this->ContactsList->currentRow();
  delete this->ContactsList->currentItem();
  this->ContactsList->setCurrentRow(pos - 1);
}

QString ClientGUI::getSelected() const
{
  if (!this->ContactsList->currentItem())
    return "";
  return this->ContactsList->currentItem()->text();
}

const QString ClientGUI::getInitialContactName() const
{
  if (!this->_addContact)
    return *new QString("");// TODO throw a fatal exception
  return this->_addContact->getInitialContactName();
}

QString ClientGUI::getNewContactName() const
{
  if (!this->_addContact)
    return *new QString("");// TODO throw a fatal exception
  return this->_addContact->getNewContactName();
}

QString ClientGUI::getNewContactKey() const
{
  if (!this->_addContact)
    return *new QString("");// TODO throw a fatal exception
  return this->_addContact->getNewContactKey();
}

QString ClientGUI::getRootName() const
{
  if (!this->_rootNode)
    return *new QString("");// TODO throw a fatal exception
  return this->_rootNode->getRootName();
}

QString ClientGUI::getRootKey() const
{
  if (!this->_rootNode)
    return *new QString("");// TODO throw a fatal exception
  return this->_rootNode->getRootKey();
}

QString ClientGUI::getRootIP() const
{
  if (!this->_rootNode)
    return *new QString("");// TODO throw a fatal exception
  return this->_rootNode->getRootIP();
}




void ClientGUI::createAddContact(const QString& name, const QString &key) {
  if (_addContact)
    return ;
  _addContact = new addContactGui(this->_controller, *this, name, key);

  QObject::connect(_addContact, SIGNAL(destroyed()), this, SLOT(addContactDeleted()));
  _addContact->show();
}



void ClientGUI::createAddContact() {
  if (_addContact)
    return ;
  _addContact = new addContactGui(this->_controller, *this);

  QObject::connect(_addContact, SIGNAL(destroyed()), this, SLOT(addContactDeleted()));
  _addContact->show();
}

void ClientGUI::deleteAddContact() {
  delete _addContact;
}

void ClientGUI::deleteRootNode() {
  delete _rootNode;
}

void ClientGUI::addContactDeleted() {
    _addContact = 0;
}

void ClientGUI::addContact(const QString &str)
{
  this->ContactsList->addItem(str);
}

// void ClientGUI::addContact(Contact* c) {
//     QString str;
//     str.append(c->getName().c_str());
//     this->ContactsList->addItem(str);
// }

void ClientGUI::createRootNodeGui(const QString& name, const QString &key, const QString &IP) {
  if (_rootNode)
    return ;
  _rootNode = new rootNodeGui(this->_controller, *this, name, key, IP);

  QObject::connect(_rootNode, SIGNAL(destroyed()), this, SLOT(rootNodeGuiDeleted()));
  _rootNode->show();
}

void ClientGUI::rootNodeGuiDeleted() {
    _rootNode = 0;
}
