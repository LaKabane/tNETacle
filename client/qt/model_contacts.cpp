#include <QDebug>
#include "exception.h"
#include "model_contacts.h"

const QString ModelContacts::_name = "Contacts";

ModelContacts::ModelContacts(Controller &controller):
  _contacts(),
  _controller(controller)
{

}

const QString &ModelContacts::getObjectName() const
{
  return ModelContacts::_name;
}

const QMap<QString, QString> &ModelContacts::getData() const
{
  return _contacts;
}

void ModelContacts::addContact(const QString &name, const QString &key)
{
  if (!name.length())
    throw new Exception("No name");
  if (_contacts.contains(name))
    throw new Exception("Name already exist");
   _contacts[name] = key;
   qDebug() << this->toJson();
}

const QString &ModelContacts::getKey(const QString &name)
{
  if (!_contacts.contains(name))
    throw new Exception("Name does not exist");
  return _contacts[name];
}

void ModelContacts::delContact(const QString &name)
{
  if (_contacts.remove(name) != 1)
    throw new Exception("Name does not exist");
}

void  ModelContacts::editContact(const QString &old, const QString &name, const QString &key)
{
  this->delContact(old);
  this->addContact(name, key);
}
